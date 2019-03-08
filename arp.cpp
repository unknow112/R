#include "arp.hpp"
#include "traffic.hpp"
#include <tins/network_interface.h>
#include <vector>
#include <unordered_map>
#include <tins/arp.h>
#include <tins/hw_address.h>
#include <tins/ip.h>
#include <tins/ethernetII.h>
#include <tins/pdu.h>
#include <iostream>
#include <cstdint>

Arp::Arp(QObject *parent) : QObject(parent)
{
    timer_.setSingleShot(true);
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

void Arp::LookupIP(const std::string& s_dst_ip, const std::string& out_intf )
{
    Tins::IPv4Address dst_ip(s_dst_ip);
    std::cerr<< "got to lookup " << dst_ip.to_string() << '\n';
    ArpTable& m_t = arp_tables_[out_intf];
    auto res = m_t.mappings_.find(dst_ip);
    if (m_t.mappings_.end() == res){
        // check if first arp request
       // if (m_t.unresolved_.end() != m_t.unresolved_.find(dst_ip)){ // buffering je nejaky vadny, chek it out
            // create request and send it, set timer
            auto t = CreateRequest(dst_ip, out_intf);
            m_t.unresolved_.insert({dst_ip, {}});
            emit SendArpFrame(t);
        //}
    } else {
        // idk daco, si asi vlastne callbacky naimplementujem
    }
}

void Arp::processArp(const Traffic& t )
{
    Tins::ARP a = t.frame_->rfind_pdu<Tins::ARP>();
    auto type = a.opcode();
    switch(type){
    case ARP_OP_REQUEST:
        if (arp_tables_[t.in_intf_].my_ip_ == a.target_ip_addr()){
            SendArpFrame(CreateReply(t));
            std::cerr << "ARP SHOULD REPLY TO REQUEST\n";
        }
    break;
    case ARP_OP_REPLY:
        ArpTable& m_t = arp_tables_[t.in_intf_];
        auto buffered = m_t.unresolved_[a.sender_ip_addr()];
        m_t.unresolved_.erase(a.sender_ip_addr());
        m_t.mappings_.insert({a.sender_ip_addr(), a.sender_hw_addr()});
        emit ArpTableChanged();
        //send away all buffered
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
    m_t.mappings_.clear();
    m_t.mappings_.insert({ip, m_t.my_mac_ });
    m_t.unresolved_.clear();
    emit ArpTableChanged();
}

ArpTable& Arp::fooGetTable()
{
    return (*arp_tables_.begin()).second;
}
