#include "outinterface.hpp"
#include <tins/network_interface.h>
#include <tins/packet_sender.h>
#include <vector>
#include "traffic.hpp"
OutInterface::OutInterface(QObject *parent) : QObject(parent)
{

}

void OutInterface::setInterfaces(const std::vector<Tins::NetworkInterface>& intfs)
{
    d_ = Tins::PacketSender{};
    for (const auto& x: intfs){
        devices_.insert({x.name(), Tins::PacketSender(x)});
    }
}
void OutInterface::sendTraffic(Traffic t)
{
    devices_[t.out_intf_].send(*t.frame_);
}
