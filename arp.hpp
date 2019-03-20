#ifndef ARP_HPP
#define ARP_HPP
#include <cstdint>
#include <unordered_map>
#include <tins/ip.h>
#include "traffic.hpp"
#include <QObject>
#include <tins/hw_address.h>
#include <vector>
#include <tins/ip_address.h>
#include <QTimer>
#include <string>
#include "interfaceip.hpp"
#include <tins/network_interface.h>
#include <qlineedit.h>
#include <qtablewidget.h>
using MACAddress= Tins::HWAddress<6>;
const uint16_t ARP_OP_REQUEST = 1;
const uint16_t ARP_OP_REPLY = 2;

struct ArpViewMapItem
{
    ArpViewMapItem(QLineEdit* lab, QTableWidget* tab):
        label_(lab),
        table_(tab)
    {}
    QLineEdit& lab(){
        if (label_ == nullptr){
            throw std::runtime_error("uninitialized view, panic");
        }
        return *label_;
    }
    QTableWidget& tab(){
        if (table_ == nullptr){
            throw std::runtime_error("uninitialized view, panic");
        }
        return *table_;
    }
    ArpViewMapItem() = default;
    QLineEdit *label_ = nullptr;
    QTableWidget *table_ = nullptr;
};

struct ArpTable
{
    explicit ArpTable(const Tins::NetworkInterface& intf):
        intf_(intf.name()),
        my_mac_(intf.hw_address())
    {}
    ArpTable() = default;


    std::string intf_;
    Tins::IPv4Address my_ip_;
    Tins::IPv4Address my_net_;
    Tins::IPv4Address my_bcast_;
    uint8_t my_prefix_size_;
    MACAddress my_mac_;
    std::unordered_map<Tins::IPv4Address, MACAddress> mappings_;
};



class Arp : public QObject
{
    Q_OBJECT
public:
    explicit Arp(QObject *parent = nullptr);

    void setInterfaces(const std::vector<Tins::NetworkInterface>&);

    Traffic CreateReply(const Traffic&);
    Traffic CreateRequest(const Tins::IPv4Address& ,  const std::string&);


//    void addArpEntry();

    ArpTable& fooGetTable();
    ArpTable& GetTable(std::string);

signals:
    void ArpTableChanged(std::string);
    void SendArpFrame(Traffic);
    void SendTraffic(Traffic);

public slots:
    void processArp(const Traffic&);
    void setIP(const Tins::IPv4Address&, const std::string&, uint8_t);
    void SetIP(std::string , IPInfo);
    void LookupIP(const std::string&, const std::string&  );
    void LookupIP(Traffic t);

private:
    QTimer timer_;

    std::unordered_map<std::string, ArpTable> arp_tables_;
};

#endif // ARP_HPP
