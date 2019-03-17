#include "re.hpp"

RE::RE(QObject *parent) : QObject(parent)
{

}


void RE::SetIP(std::string intf , IPInfo ipinfo)
{
    forward_table_.erase(
        std::remove_if(
            forward_table_.begin(),
            forward_table_.end(),
            [intf](const ForwardEntry& a){return a.second.origin_ != CONN || a.second.exit_intf_ == intf ; }
        ),
        forward_table_.end()
    );
    forward_table_.push_back({ipinfo, ExitInfo(intf, CONN)});

    emit RouteTableChanged();
}

const std::vector<ForwardEntry>& RE::GetTable() const
{
    return forward_table_;
}
