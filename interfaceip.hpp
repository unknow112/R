#ifndef INTERFACEIP_HPP
#define INTERFACEIP_HPP
#include <vector>
#include <unordered_map>
#include <tins/network_interface.h>
#include <QObject>
#include <string>
#include <tins/ip_address.h>
struct IPInfo{
    IPInfo(const std::string & ip, const std::string prefix):
        ip_(ip),
        pref_l_(uint8_t(std::stol(prefix)))
    {}
    IPInfo():
        ip_("0.0.0.0"),
        pref_l_(0)
    {}
    IPInfo(const Tins::IPv4Address& ip, uint8_t pref):
        ip_(ip),
        pref_l_(pref)
    {}

    Tins::IPv4Address ip_;
    uint8_t pref_l_;
};


class InterfaceIP : public QObject
{
    Q_OBJECT
public:
    explicit InterfaceIP(QObject *parent = nullptr);
    void setInterfaces(const std::vector<Tins::NetworkInterface>&);
signals:
    void ChangeIP(std::string , IPInfo);

public slots:
    void SetIP(const std::string&, const std::string&, const std::string&);

private:
    std::unordered_map<std::string, IPInfo> i;
};

#endif // INTERFACEIP_HPP
