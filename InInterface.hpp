#ifndef IN_INTERFACE_HPP
#define IN_INTERFACE_HPP
#include <tins/network_interface.h>
#include <QObject>
#include <tins/tins.h>
#include <QThread>
#include <string>
#include "traffic.hpp"


//void interface_in(Tins::NetworkInterface&& );
class InInterface : public QThread
{
    Q_OBJECT
public:
    InInterface() = delete;
    InInterface(const InInterface&) = delete;
    InInterface(InInterface&&);
    explicit InInterface(const Tins::NetworkInterface& intf );
    void run();

signals:
    void ArpRecieved(const Traffic&);
    void TrafficRecieved(const Traffic&);
    void RipRecieved(const Traffic&);


private:
    Tins::SnifferConfiguration  makeConf(const Tins::NetworkInterface& intf);

    Tins::Sniffer device_ ;
    std::string interface_name_;
};
#endif //IN_INTERFACE_HPP
