#include "arp.hpp"
#include "traffic.hpp"
#include "interfaceip.hpp"
#include <tins/network_interface.h>
#include <vector>
#include <unordered_map>
#include <tins/arp.h>
#include <tins/hw_address.h>
#include <tins/ip.h>
#include <tins/ip_address.h>
#include <tins/ethernetII.h>
#include <tins/pdu.h>
#include <iostream>
#include <cstdint>

Arp::Arp(QObject *parent) : QObject(parent)
{
    QObject::connect(&timer_,SIGNAL(timeout()), this,SLOT(DecrementLife()));
    timer_.start(1000);
}

void Arp::WillClearArpTable(std::string intf)
{
    if (arp_tables_.find(intf) == arp_tables_.end()){
        return;
    }

    ArpTable& m_t = arp_tables_[intf];

    m_t.mappings_.clear();
    m_t.mappings_.insert({m_t.my_ip_, m_t.my_mac_ });

    m_t.life_ = {};

    emit ArpTableChanged(intf);
}

void Arp::DecrementLife()
{
    //ArpTable
    for (auto& x: arp_tables_){
        auto& arp_t = x.second;
        // prerotuj life_ a ked dekrementujes na 0, tak premaz, ez
        for (auto& y: arp_t.life_){
            y.second --;
            if (y.second == 0){
                auto elem = arp_t.mappings_.find(y.first);
                if ( elem == arp_t.mappings_.end()){
                    throw std::runtime_error("mazana ip nieje v arp lol");
                }
                arp_t.mappings_.erase(elem);
            }
        }
        emit ArpTableChanged(x.first);
    }
}


void Arp::setRE(RE * r)
{
    routing_ = r;
}


Traffic Arp::CreateReply(const Traffic& t_in)
{
    Traffic out;
    out.out_intf_ = t_in.in_intf_;
    auto x = Tins::ARP::make_arp_reply(
                t_in.frame_->rfind_pdu<Tins::ARP>().sender_ip_addr(),
                t_in.frame_->rfind_pdu<Tins::ARP>().target_ip_addr(),
                t_in.frame_->rfind_pdu<Tins::ARP>().sender_hw_addr(),
                arp_tables_[t_in.in_intf_].my_mac_
    );

    out.frame_ = std::make_shared<Tins::EthernetII>(std::move(x));
    return out;
}

Traffic Arp::CreateRequest(const Tins::IPv4Address& dst_ip,  const std::string& out_intf)
{
    std::cerr<< "creating request for arp\n";
    Traffic out;
    out.out_intf_=out_intf;
    auto x = Tins::ARP::make_arp_request(
                dst_ip,
                arp_tables_[out_intf].my_ip_,
                arp_tables_[out_intf].my_mac_
    );
    out.frame_ = std::make_shared<Tins::EthernetII>(std::move(x));
    return out;
}


Tins::IPv4Address get_network(const Tins::IPv4Address& ip, uint8_t pref_l)
{
    return Tins::IPv4Address(uint32_t(ip) & Tins::IPv4Address::from_prefix_length(pref_l));

}

Tins::IPv4Address get_broadcast(const Tins::IPv4Address& ip, uint8_t pref_l)
{

    return Tins::IPv4Address(uint32_t(get_network(ip, pref_l)) + ~uint32_t(Tins::IPv4Address::from_prefix_length(pref_l)));
}


void Arp::LookupIP(const std::string& s_dst_ip, const std::string& out_intf )
{
    Tins::IPv4Address dst_ip(s_dst_ip);
    std::cerr<< "got to lookup " << dst_ip.to_string() << '\n';
    ArpTable& m_t = arp_tables_[out_intf];
    if (m_t.my_net_ == s_dst_ip || m_t.my_bcast_ == s_dst_ip){
        std::cerr<< "nah\n";
        return;
    }
    auto res = m_t.mappings_.find(dst_ip);
    if (m_t.mappings_.end() == res){
        // create request and send it
        auto t = CreateRequest(dst_ip, out_intf);
        emit SendArpFrame(t);
    }
}

void Arp::LookupIP(Traffic t)
{
    ArpTable& m_t = arp_tables_[t.out_intf_];
    if (t.next_hop_ == m_t.my_net_ || t.next_hop_ == m_t.my_bcast_){
        return;
    }
    auto res = m_t.mappings_.find(t.next_hop_);
    if (m_t.mappings_.end() == res){
        auto t_a = CreateRequest(t.next_hop_, t.out_intf_);
        emit SendArpFrame(t_a);
    } else {
        auto & eth = t.frame_->rfind_pdu<Tins::EthernetII>();
        eth.dst_addr((*res).second);
        eth.src_addr(m_t.my_mac_);
        emit SendTraffic(t);
    }

}

void Arp::WillSendRip(Traffic t)
{
    auto payload = t.frame_->clone();
    t = Traffic(Tins::EthernetII("01:00:5E:00:00:09", arp_tables_[t.out_intf_].my_mac_) / *payload, t.out_intf_);
    emit SendTraffic(t);
}


void Arp::processArp(const Traffic& t )
{
    Tins::ARP a = t.frame_->rfind_pdu<Tins::ARP>();
    auto type = a.opcode();
    if (type == ARP_OP_REQUEST){
        if (arp_tables_[t.in_intf_].my_ip_ == a.target_ip_addr()){
            SendArpFrame(CreateReply(t));
            std::cerr << "ARP SHOULD REPLY TO REQUEST\n";
            return;
        }
        auto path = (*routing_).route(a.target_ip_addr());
        if (path.origin_ == CONN && path.exit_intf_ == t.in_intf_){
            return;
        }
        if (path.exit_intf_ != "null"){
            SendArpFrame(CreateReply(t));
            std::cerr << "PROXYARP SHOULD REPLY TO REQUEST\n";
            return;
        }
   }
    if (type == ARP_OP_REPLY){
        ArpTable& m_t = arp_tables_[t.in_intf_];
        m_t.mappings_.insert({a.sender_ip_addr(), a.sender_hw_addr()});
        m_t.life_.insert({a.sender_ip_addr(), max_age_});
        emit ArpTableChanged(t.in_intf_);
        return;
    }
}

void Arp::setInterfaces(const std::vector<Tins::NetworkInterface>& intfs)
{
    for (const auto& intf : intfs) {
        arp_tables_.insert({intf.name(), ArpTable(intf.name())});
    }
}

void Arp::setIP(const Tins::IPv4Address& ip, const std::string& intf, uint8_t prefix)
{
    ArpTable& m_t = arp_tables_[intf];
    if (m_t.my_ip_ == ip && m_t.my_prefix_size_ == prefix){
        return;
    }
    m_t.my_ip_ = ip;
    m_t.my_prefix_size_ = prefix;
    m_t.my_net_ = get_network(ip, prefix);
    m_t.my_bcast_ = get_broadcast(ip, prefix);
    WillClearArpTable(intf);

}

void Arp::SetIP(std::string intf , IPInfo ipinfo){
    setIP(ipinfo.ip_, intf, ipinfo.pref_l_);
}


ArpTable& Arp::fooGetTable()
{
    return (*arp_tables_.begin()).second;
}

ArpTable& Arp::GetTable(std::string intf)
{
    auto result = arp_tables_.find(intf);
    if (result == arp_tables_.end()){
        throw std::runtime_error("Arp::GetTable(std::string) got invalid intf '"+intf+"' in internal call");
    }
    return (*result).second;
}
