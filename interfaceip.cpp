#include "interfaceip.hpp"
#include <vector>
#include <tins/network_interface.h>
#include <unordered_map>
#include <iostream>
#include <tins/exceptions.h>
InterfaceIP::InterfaceIP(QObject *parent) : QObject(parent)
{

}

void InterfaceIP::setInterfaces(const std::vector<Tins::NetworkInterface>& intfs)
{
    for(const auto& x: intfs){
        i.insert({x.name(),{}});
    }
}


bool is_network(const Tins::IPv4Address& ip, const Tins::IPv4Address& mask){
    return (uint32_t(ip) & uint32_t(mask)) == uint32_t(ip) ;
}

bool is_broadcast(const Tins::IPv4Address& ip, const Tins::IPv4Address& mask){
    auto net = (uint32_t(ip) & uint32_t(mask));
    auto bbits = ~uint32_t(mask);
    return Tins::IPv4Address(net + bbits) == ip;
}

void InterfaceIP::SetIP(const std::string& intf, const std::string& ip, const std::string& pref)
{
    if (i.find(intf) == i.end()){
        std::cerr << "Cannot change ip on " << intf<< " - intf doesnt exist!\n";
        return;
    }

    Tins::IPv4Address new_ip_;

    try {
        new_ip_ = Tins::IPv4Address(ip);
    } catch (Tins::invalid_address) {
        std::cerr << "Cannot change ip on " << intf<< ", " << ip << " - is invalid address!\n";
        return;
    }

    long new_pref_l = 0;
    try {
        new_pref_l = std::stol(pref);
    } catch (std::invalid_argument) {
        std::cerr << "Cannot change ip on " << intf<< ", " << pref << " - is not valid prefix lenght!\n";
        return;
    }

    if (new_pref_l > 30 || new_pref_l < 1){
        std::cerr << "Cannot change ip on " << intf<< ", " << pref << " - is not valid prefix lenght!\n";
        return;
    }
    auto mask = Tins::IPv4Address::from_prefix_length(uint32_t(new_pref_l));
    if ( new_ip_.is_loopback() ||
         new_ip_ >= Tins::IPv4Address("224.0.0.0") ||
         is_broadcast(new_ip_, mask) ||
         is_network(new_ip_, mask)
       ){
        std::cerr<< "\n\nbeep\n\n";
        std::cerr << "Cannot change ip on " << intf<< ", " << ip << " - address from invalid range!\n";
        return;
    }

    auto new_info = IPInfo(new_ip_, uint8_t(new_pref_l));
    //auto old_info = *(i.find(intf));


    i[intf] = new_info;
    // check overlaps in the future
    emit ChangeIP(intf, new_info);
}

std::unordered_map<std::string, IPInfo> InterfaceIP::data() const
{
    return i;
}
