#ifndef OUTINTERFACE_HPP
#define OUTINTERFACE_HPP
#include <unordered_map>
#include <tins/packet_sender.h>
#include <QObject>
#include "traffic.hpp"

class OutInterface : public QObject
{
    Q_OBJECT
public:
    explicit OutInterface(QObject *parent = nullptr);
    void setInterfaces(const std::vector<Tins::NetworkInterface>& intfs);

signals:

public slots:
    void sendTraffic(Traffic);


private:
    std::unordered_map<std::string, Tins::PacketSender> devices_;
    Tins::PacketSender d_;
};

#endif // OUTINTERFACE_HPP
