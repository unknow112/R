#ifndef INTERFACEIP_HPP
#define INTERFACEIP_HPP
#include <vector>
#include <unordered_map>
#include <tins/network_interface.h>
#include <QObject>
#include <string>
#include <tins/ip_address.h>
#include "ip_helpers.hpp"
bool is_network(const Tins::IPv4Address& ip, const Tins::IPv4Address& mask);




class InterfaceIP : public QObject
{
    Q_OBJECT
public:
    explicit InterfaceIP(QObject *parent = nullptr);
    void setInterfaces(const std::vector<Tins::NetworkInterface>&);
    std::unordered_map<std::string, IPInfo> data() const;

signals:
    void ChangeIP(std::string , IPInfo);

public slots:
    void SetIP(const std::string&, const std::string&, const std::string&);

private:
    std::unordered_map<std::string, IPInfo> i;
};

#endif // INTERFACEIP_HPP
