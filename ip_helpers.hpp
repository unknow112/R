#ifndef IP_OBJECTS_HPP
#define IP_OBJECTS_HPP
#include <utility>
#include <tins/ip_address.h>
#include <cstdint>

using Prefix = std::pair<Tins::IPv4Address, uint16_t>;

enum RouteSource{
    CONN,
    STATIC,
    RIP,
    IMPLICIT
};

struct IPInfo{
    IPInfo(const std::string & ip, const std::string prefix):
        ip_(ip),
        pref_l_(uint16_t(std::stol(prefix)))
    {}
    IPInfo():
        ip_("0.0.0.0"),
        pref_l_(0)
    {}
    IPInfo(const Tins::IPv4Address& ip, uint16_t pref):
        ip_(ip),
        pref_l_(pref)
    {}

    bool operator==(const IPInfo& o)
    {
        return ip_ == o.ip_ && pref_l_ == o.pref_l_;
    }


    Tins::IPv4Address ip_;
    uint16_t pref_l_;
};


struct ExitInfo
{
    ExitInfo(std::string intf, Tins::IPv4Address next_hop, RouteSource origin):
        exit_intf_(intf),
        next_hop_(next_hop),
        origin_(origin)
    {}
    ExitInfo(std::string intf, RouteSource origin):
        exit_intf_(intf),
        origin_(origin)
    {}
    ExitInfo() = default;

    std::string exit_intf_;
    Tins::IPv4Address next_hop_;
    RouteSource origin_;
    int weight_ = 0;
};

using PrefixInfo = IPInfo;

using ForwardEntry = std::pair<PrefixInfo, ExitInfo >;
bool match_prefix(const Tins::IPv4Address& dst, const PrefixInfo& prefix);

#endif // IP_OBJECTS_HPP
