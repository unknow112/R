#ifndef RE_HPP
#define RE_HPP
#include "ip_helpers.hpp"
#include "traffic.hpp"
#include <QObject>
#include "interfaceip.hpp"
#include "interfaceip.hpp"
#include <optional>
#include "tins/ip_address.h"
#include "rip.hpp"



class RE : public QObject
{
    Q_OBJECT
public:
    explicit RE(QObject *parent = nullptr);
    const std::vector<ForwardEntry>& GetTable() const;
    ExitInfo route(const Tins::IPv4Address& dst);
    void setRipEngine(Rip * ptr)
    {
        rip_e_ = ptr;
    }

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
    void Rebuild();


private:
    //bool match_prefix(const Tins::IPv4Address& dst, const PrefixInfo& prefix);
    const ForwardEntry implicit_{PrefixInfo("0.0.0.0", "0"), ExitInfo("null", IMPLICIT)};
    std::vector<ForwardEntry > forward_table_;
    std::vector<ForwardEntry > static_routes_;
    void TryAdd(ForwardEntry);
    bool is_up(const std::string& );

    Rip *rip_e_ = nullptr;

};

#endif // RE_HPP
