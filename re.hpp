#ifndef RE_HPP
#define RE_HPP

#include "traffic.hpp"
#include <QObject>
#include "interfaceip.hpp"
#include "interfaceip.hpp"
#include <optional>
#include "tins/ip_address.h"

enum RouteSource{
    CONN,
    STATIC,
    RIP,
    IMPLICIT
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


class RE : public QObject
{
    Q_OBJECT
public:
    explicit RE(QObject *parent = nullptr);
    const std::vector<ForwardEntry>& GetTable() const;
    ExitInfo route(const Tins::IPv4Address& dst);

signals:
    void RedrawTable();
    void RouteTableChanged();
    void SendTraffic(Traffic);
    void PrintingDone();

public slots:
    void SetIP(std::string, IPInfo);
    void RouteTraffic(const Traffic&);
    void PrintStatic();
    void AddStatic(ForwardEntry);
    void DelStatic(int);

private:
    bool match_prefix(const Tins::IPv4Address& dst, const PrefixInfo& prefix);
    const ForwardEntry implicit_{PrefixInfo("0.0.0.0", "0"), ExitInfo("null", IMPLICIT)};
    std::vector<ForwardEntry > forward_table_;
    std::vector<ForwardEntry > static_routes_;
    void TryAdd(ForwardEntry);
    bool is_up(const std::string& );
    void Rebuild();

};

#endif // RE_HPP
