#ifndef RIP_HPP
#define RIP_HPP
#include "traffic.hpp"
#include <QObject>
#include "interfaceip.hpp"
#include <tins/rawpdu.h>
#include <tins/ip_address.h>
#include <QTimer>
#include <tins/udp.h>
#include "ip_helpers.hpp"


const uint32_t RIP_INVALID_TIMER = 35;
const uint32_t RIP_FLUSH_TIMER = 60;
const uint32_t RIP_HOLDDOWN_TIMER = 10;

enum RipUpdateState
{
    OK,
    HOLDDOWN,
    POSSIBLY_DOWN
};


class RipUpdate
{
public:
    RipUpdate(const Tins::RawPDU::payload_type&, std::string, Tins::IPv4Address, ushort);

    RipUpdate(Tins::IPv4Address dst_pref, uint16_t dst_prefl, const std::string& origin):
        metric_(0),
        origin_intf_(origin)
    {
        dst_prefix_ = {dst_pref, dst_prefl};
    }
    ForwardEntry to_fwentry() const;

    void reset_timers();
    void decrement_timers();

    void serialize_to(Tins::RawPDU::payload_type& fillme) const;
    Tins::UDP triggered_update() const;

    bool operator==(const RipUpdate&);
    bool operator!=(const RipUpdate& o)
    {
        return !(*this == o);
    }


    Prefix dst_prefix_;
    uint16_t metric_;
    std::string origin_intf_;
    Tins::IPv4Address origin_ip_ = "0.0.0.0";

    RipUpdateState state_ = OK ;

    uint32_t invalid_timer_ = RIP_INVALID_TIMER;
    uint32_t flush_timer_ = RIP_FLUSH_TIMER;
    uint32_t holddown_timer_ = 0;

};

class Rip : public QObject
{
    Q_OBJECT
public:
    explicit Rip(QObject *parent = nullptr);
    void SetIpObjs(InterfaceIP* );
    const std::vector<RipUpdate >& getDataBase() const;
    int getTimeUpdate() const    {
        return int(update_timer_.remainingTime()/1000);
    }
    bool isOnSomewhere() const    {
        return !db_locals_.empty();
    }

signals:
    void DoSendRip(Traffic);
    void RipDatabaseChanged();
    void RipDatabaseTimersChanged();


public slots:
    void ProcessUpdate(const Traffic&);
    void WillToggleRip(bool, std::string);
    void SendTriggeredUpdate(RipUpdate);
    void SetIP(std::string , IPInfo);

private slots:
    void SendPeriodicUpdate();
    void UpdateLife();
    void SendPoisionUpdate(RipUpdate);
    void SendPoisionUpdate(RipUpdate, std::string);

private:
    void insert_update(RipUpdate&&);
    void purge_intf(std::string);


    InterfaceIP * intf_ip_ = nullptr;
    std::unordered_map<std::string, bool > is_on_;
    std::vector<RipUpdate > db_locals_;
    std::vector<RipUpdate > db_remotes_;

    QTimer update_timer_;
    QTimer clock_;
};

#endif // RIP_HPP
