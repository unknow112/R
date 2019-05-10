#include "ip_helpers.hpp"

bool match_prefix(const Tins::IPv4Address& dst, const PrefixInfo& prefix)
{
    auto mask = uint32_t(Tins::IPv4Address::from_prefix_length(prefix.pref_l_));
    auto net = Tins::IPv4Address(uint32_t(dst) & mask);
    return net == prefix.ip_;
}
