#include "re.hpp"
#include <tins/ip.h>
#include <iostream>
#include "ip_helpers.hpp"
#include <exception>
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

    Rebuild();

    emit RouteTableChanged();
}

const std::vector<ForwardEntry>& RE::GetTable() const
{
    return forward_table_;
}



ExitInfo RE::route(const Tins::IPv4Address& dst)
{
    auto best_match = forward_table_.end();
    auto current = forward_table_.begin();
    while (current != forward_table_.end()){
        if (match_prefix(dst, (*current).first)){
            best_match = current;
            break;
        }
        current++;
    }

    for(; current != forward_table_.end() ; current++ ){
        if (match_prefix(dst, (*current).first)){
            if ((*current).first.pref_l_ > (*best_match).first.pref_l_){
                best_match = current;
            }
        }
    }

    if (best_match == forward_table_.end()){
        return implicit_.second;
    } else {
        return (*best_match).second;
    }
}

void RE::PrintStatic()
{
    std::cout << "All configured static routes:\n";
    int index = 0;
    for (const auto& x: static_routes_ ){
        std::cout << index <<"  "<< x.first.ip_.to_string() << '/' << uint32_t(x.first.pref_l_) << " via ";
        if (x.second.next_hop_ != "0.0.0.0"){
            std::cout << "  "<< x.second.next_hop_.to_string();
        }
        if (x.second.exit_intf_ != ""){
            std::cout << "  "<< x.second.exit_intf_;
        }
        std::cout << '\n';
        index++;
    }
    emit PrintingDone();
}

bool is_better_source(RouteSource a, RouteSource b)
{
    switch(a){
    case CONN:
        return true;
    case STATIC:
        return b != CONN;
    case RIP:
        return b == IMPLICIT;
    case IMPLICIT:
        return false;
    }
    throw std::runtime_error("unhandled enum in is_better_source");

}

bool RE::is_up(const std::string& intf)
{
    for (const auto& x:forward_table_ ){
        if (x.second.exit_intf_ == intf && x.second.origin_ == CONN){
            return true;
        }
    }
    return false;
}

void RE::TryAdd(ForwardEntry e)
{
    std::optional<ForwardEntry> orig;
    //0. ak uz mam taky prefix s lepsim AD tak kaslat
    for (auto x = forward_table_.begin(); x != forward_table_.end(); x++) {
        if (e.first == (*x).first){
            if  (is_better_source((*x).second.origin_, e.second.origin_)){
                return;
            } else {
                // treba vymazat bo pride daco lepsie , iteracia sa devaliduje !!!
                orig = *x;
                forward_table_.erase(x);
                break;
            }
        }
    }
    //1. ak je oboje tak musi byt dst routovane tym intf co som tam hodil
    if (e.second.next_hop_ != "0.0.0.0" && e.second.exit_intf_ != ""){
        if (route(e.second.next_hop_).exit_intf_ == e.second.exit_intf_){
            forward_table_.push_back(e);
        }
    //2. ak iba next hop, ziti cim  je routovane a ci vobec je routovatelne
    } else if (e.second.next_hop_ != "0.0.0.0"){
        auto dir = route(e.second.next_hop_);
        if (dir.origin_ == CONN){
            e.second.exit_intf_ = dir.exit_intf_;
            forward_table_.push_back(e);
        }
    //3. ak je exit intf tak proste to tam picni. ak je inft up
    } else if (e.second.exit_intf_ != ""){
        if (is_up(e.second.exit_intf_)){
            forward_table_.push_back(e);
        }
    } else if(orig.has_value()){
        TryAdd(orig.value());
    }

    //4. emitni zmenu
    emit RouteTableChanged();

}

void RE::Rebuild()
{
    std::vector<ForwardEntry> new_ft{implicit_};
    std::for_each(
        forward_table_.begin(),
        forward_table_.end(),
        [&new_ft](const ForwardEntry& x){
            if (x.second.origin_ == CONN){
                new_ft.push_back(x);
            }
        }
    );
    std::swap(new_ft, forward_table_);
    std::for_each(
        static_routes_.begin(),
        static_routes_.end(),
        [this](const ForwardEntry& x){
            (*this).TryAdd(x);
        }
    );

    auto & ripdb = rip_e_->getDataBase();
    std::for_each(
        ripdb.begin(),
        ripdb.end(),
        [this](const RipUpdate& a){
            if (a.metric_ < 16) {
                (*this).TryAdd(a.to_fwentry());
            }
        }
    );
    emit RouteTableChanged();
}

void RE::DelStatic(int index)
{
    if (size_t(index) >= static_routes_.size()){
        std::cout << "index too high to del\nrefer to 'ip route show' command\n";
    } else {
        auto elem = static_routes_.begin();
        elem += index;
        static_routes_.erase(elem);
        Rebuild();
        emit RouteTableChanged();
    }
    // proste zavolaj rebuild tabulky a napchaj ich tam od znova bez toho vymazaneho idk idc.

    emit PrintingDone();
}

void RE::AddStatic(ForwardEntry x)
{
    static_routes_.push_back(x);
    TryAdd(x);
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
    if (direction.next_hop_ == "0.0.0.0"){
        out.next_hop_ = packet.dst_addr();
    } else {
        out.next_hop_ = direction.next_hop_;
    }

    emit SendTraffic(out);

}
