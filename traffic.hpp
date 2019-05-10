#ifndef TRAFFIC_HPP
#define TRAFFIC_HPP
#include <string>
#include <tins/pdu.h>
#include <memory>
#include <tins/ip_address.h>

class Traffic
{
public:
    Traffic() = default;
    Traffic(const std::string& in_intf, const Tins::PDU& frame):
        frame_(frame.clone()),
      in_intf_(in_intf)
    {}

    Traffic(const Tins::PDU& frame, const std::string& out_intf):
        frame_(frame.clone()),
        out_intf_(out_intf)
    {}



    std::shared_ptr<Tins::PDU> frame_;
    std::string in_intf_;
    std::string out_intf_;
    Tins::IPv4Address next_hop_;

};

#endif // TRAFFIC_HPP
