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
    auto new_pref_l = std::stol(pref);
    if (new_pref_l > 30 || new_pref_l < 1){
        std::cerr << "Cannot change ip on " << intf<< ", " << pref << " - is not valid prefix lenght!\n";
        return;
    }

    auto new_info = IPInfo(new_ip_, uint8_t(new_pref_l));
    //auto old_info = *(i.find(intf));

    i[intf] = new_info;
    // check overlaps in the future
    emit ChangeIP(intf, new_info);
}
