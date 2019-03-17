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
        return *label_;
    }
    QTableWidget& tab(){
        return *table_;
    }
    QLineEdit *label_;
    QTableWidget *table_;
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

public slots:
    void processArp(const Traffic&);
    void setIP(const Tins::IPv4Address&, const std::string&, uint8_t);
    void SetIP(std::string , IPInfo);
    void LookupIP(const std::string&, const std::string&  );


private:
    QTimer timer_;

    std::unordered_map<std::string, ArpTable> arp_tables_;
};

#endif // ARP_HPP
