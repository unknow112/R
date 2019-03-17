#ifndef RE_HPP
#define RE_HPP

#include "traffic.hpp"
#include <QObject>
#include "interfaceip.hpp"
#include "interfaceip.hpp"
#include "tins/ip_address.h"

enum RouteSource{
    CONN,
    STATIC,
    RIP
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

    std::string exit_intf_;
    Tins::IPv4Address next_hop_;
    RouteSource origin_;
    int weight_ = 0;
};

using PrefixInfo = IPInfo;

using ForwardEntry = std::pair<PrefixInfo, ExitInfo >;


class RE : public QObject
{
    Q_OBJECT
public:
    explicit RE(QObject *parent = nullptr);
    const std::vector<ForwardEntry>& GetTable() const;

signals:
    void RedrawTable();
    void RouteTableChanged();

public slots:
    void SetIP(std::string, IPInfo);
    //void RouteTraffic(Traffic);


private:
    std::vector<ForwardEntry > forward_table_;
};

#endif // RE_HPP
