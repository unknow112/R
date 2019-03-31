#include "rip.hpp"
#include "interfaceip.hpp"

Rip::Rip(QObject *parent) : QObject(parent)
{

}
void Rip::setObjs(RE * r, InterfaceIP* i)
{
    routing_ = r;
    ips_ = i;
}
