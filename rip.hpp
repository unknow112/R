#ifndef RIP_HPP
#define RIP_HPP

#include <QObject>
#include "re.hpp"
#include "interfaceip.hpp"
class Rip : public QObject
{
    Q_OBJECT
public:
    explicit Rip(QObject *parent = nullptr);
    void setObjs(RE*, InterfaceIP *);

signals:

public slots:

private:
    RE * routing_;
    InterfaceIP * ips_;

};

#endif // RIP_HPP
