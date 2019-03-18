
#include "InInterface.hpp"
#include <string>
#include <tins/tins.h>
#include <tins/packet.h>
#include <tins/pdu.h>
#include <tins/arp.h>
#include <tins/hw_address.h>
#include <iostream>
#include <sstream>
#include <tins/arp.h>
#include <QObject>
#include "traffic.hpp"

InInterface::InInterface(const Tins::NetworkInterface& intf ):
        device_(intf.name(), makeConf(intf)),
        interface_name_(intf.name())
{}

InInterface::InInterface(InInterface&& other):
    device_(std::move(other.device_)),
    interface_name_(std::move(other.interface_name_))
{}


void InInterface::run()
{
    std::cerr << "started capture on "<<interface_name_ << "\n";
    for (auto &packet : device_) {
        if (nullptr != (*packet.pdu()).find_pdu<Tins::ARP>()){
            emit ArpRecieved(Traffic(interface_name_, *packet.pdu()));
            continue;
        }
        if (nullptr != (*packet.pdu()).find_pdu<Tins::IP>()){
            emit TrafficRecieved(Traffic(interface_name_, *packet.pdu()));
        }
    }
}


Tins::SnifferConfiguration InInterface::makeConf(const Tins::NetworkInterface& intf){
    Tins::SnifferConfiguration config;
    config.set_immediate_mode(true);
    config.set_promisc_mode(false);
    config.set_filter("not ether src host "+intf.hw_address().to_string());
    return config;
}


/*
class queue_pusher_functor
{
public:
    queue_pusher_functor(const std::string& in_intf, sr::ArpEngine& arp_eng):
        in_intf_(in_intf),
        arp_eng_(arp_eng)
        {}
    bool operator()(Tins::Packet&& data)
    {
        if (NULL != packed.frame_.find_pdu<Tins::ARP>())
        {
            arp_eng_.insert_event(packed.frame_.rfind_pdu<Tins::ARP>());
            continue;
        }
        
        Traffic packed{in_intf_, std::move(data.pdu())};

        //prcni packed do routing engine

        return true; //change when time to stop
    }

private:
    const std::string in_intf_;
    ArpEngine& arp_engine_;
    RouterEngine& router_engine = sr::RouterEngine::instance();
};

void interface_in(Tins::NetworkInterface&& intf, sr::ArpEngine& arp_eng)
{

    Tins::SnifferConfiguration config;
    config.set_immediate_mode(true);
    config.set_promisc_mode(false);
    config.set_filter("ether dst "+intf.hw_address().to_string());
    Tins::Sniffer device (intf.name(), config);

    device.sniff_loop(queue_pusher_functor(intf.name(),arp_eng));


}
*/
