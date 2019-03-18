#include "re.hpp"
#include <tins/ip.h>
RE::RE(QObject *parent) : QObject(parent)
{
    forward_table_.push_back(implicit_);
}

Tins::IPv4Address get_network(const IPInfo ii)
{
    return Tins::IPv4Address(
                uint32_t(ii.ip_) & uint32_t(Tins::IPv4Address::from_prefix_length(ii.pref_l_))
    );
}

void RE::SetIP(std::string intf , IPInfo ipinfo)
{
    forward_table_.erase(
        std::remove_if(
            forward_table_.begin(),
            forward_table_.end(),
            [intf](const ForwardEntry& a){
                return (a.second.origin_ != CONN && a.second.origin_ != IMPLICIT ) ||
                        a.second.exit_intf_ == intf ;
            }
        ),
        forward_table_.end()
    );
    ipinfo.ip_ = get_network(ipinfo);
    forward_table_.push_back({ipinfo, ExitInfo(intf, CONN)});

    emit RouteTableChanged();
}

const std::vector<ForwardEntry>& RE::GetTable() const
{
    return forward_table_;
}

bool RE::match_prefix(const Tins::IPv4Address& dst, const PrefixInfo& prefix)
{
    auto mask = uint32_t(Tins::IPv4Address::from_prefix_length(prefix.pref_l_));
    auto net = Tins::IPv4Address(uint32_t(dst) & mask);
    return net == prefix.ip_;
}

ExitInfo RE::route(const Tins::IPv4Address& dst)
{
    auto best_match = forward_table_.begin();

    for(auto current = forward_table_.begin() + 1 ; current != forward_table_.end() ; current++ ){
        if (match_prefix(dst, (*current).first)){
            if ((*current).first.pref_l_ > (*best_match).first.pref_l_){
                best_match = current;
            }
        }
    }
    return (*best_match).second;

}

void RE::RouteTraffic(const Traffic& in)
{
    auto old_ttl = in.frame_->rfind_pdu<Tins::IP>().ttl();
    if (old_ttl <= 1){
        return;
    }
    auto out = in;
    auto & packet = out.frame_->rfind_pdu<Tins::IP>();
    packet.ttl(old_ttl -1);
    auto direction = route(packet.dst_addr());

    if (direction.exit_intf_ == "null"){
        return;
    }

    out.out_intf_ = direction.exit_intf_;
    if (direction.origin_ == CONN){
        out.next_hop_ = packet.dst_addr();
    } else {
        out.next_hop_ = direction.next_hop_;
    }

    emit SendTraffic(out);

}
