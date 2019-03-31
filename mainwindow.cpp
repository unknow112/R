#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "get_int.hpp"
#include <tins/network_interface.h>
#include <iostream>
#include "traffic.hpp"
#include <vector>
#include <sstream>
#include <array>
#include <tins/pdu.h>
#include <string>
#include "outinterface.hpp"
#include "arp.hpp"
#include "interfaceip.hpp"


Q_DECLARE_METATYPE(ForwardEntry)
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    auto interfaces  = get_interfaces();
    qRegisterMetaType<Traffic>("Traffic");
    qRegisterMetaType<ForwardEntry>("FE");

    arp_.setInterfaces(interfaces);
    for (const auto& intf: interfaces){
        input_intf_.emplace_back(intf);
    }

    for (auto & intf: input_intf_){
        QObject::connect(
                &intf, SIGNAL(ArpRecieved(const Traffic&)),
                &arp_, SLOT(processArp(const Traffic&))
        );
        QObject::connect(
                &intf, SIGNAL(TrafficRecieved(const Traffic&)),
                &routing_e_,SLOT(RouteTraffic(const Traffic&))
        );
        intf.start();
    }

    {
        auto intf = interfaces.front();
        intf_arp_view_mapping_.insert({intf.name(), ArpViewMapItem(ui -> arp_name, ui -> arp_table_view)});
    }
    {
        auto intf = interfaces.back();
        intf_arp_view_mapping_.insert({intf.name(), ArpViewMapItem(ui -> arp_name_2, ui -> arp_table_view_2)});
    }
    for (auto& item : intf_arp_view_mapping_){
        item.second.lab().setText(item.first.data());
    }



    ip_intf_.setInterfaces(interfaces);
    output_intf_.setInterfaces(interfaces);

    QObject::connect(
                &con_, SIGNAL(ChangeIP(const std::string&, const std::string&, const std::string&)),
                &ip_intf_, SLOT(SetIP(const std::string&, const std::string&, const std::string&))
    );

    QObject::connect(
                &ip_intf_, SIGNAL(ChangeIP(std::string , IPInfo)),
                &arp_, SLOT(SetIP(std::string , IPInfo))
    );

    QObject::connect(
                &ip_intf_, SIGNAL(ChangeIP(std::string , IPInfo)),
                &routing_e_, SLOT(SetIP(std::string, IPInfo))
    );

    QObject::connect(
                &arp_,SIGNAL(ArpTableChanged(std::string)),
                this, SLOT(redrawArpTable(std::string))
    );

    QObject::connect(
                &routing_e_, SIGNAL(RouteTableChanged()),
                this, SLOT(redrawRouteTable())
    );

    QObject::connect(
                &routing_e_, SIGNAL(SendTraffic(Traffic)),
                &arp_, SLOT(LookupIP(Traffic))
    );

    QObject::connect(
                &arp_, SIGNAL(SendTraffic(Traffic)),
                &output_intf_, SLOT(sendTraffic(Traffic))
    );

    QObject::connect(
                &arp_, SIGNAL(SendArpFrame(Traffic)),
                &output_intf_, SLOT(sendTraffic(Traffic))
    );

    QObject::connect(
                &con_, SIGNAL(QueryArp(const std::string&, const std::string&)),
                &arp_, SLOT(LookupIP(const std::string&, const std::string&))
    );

    QObject::connect(
                &con_, SIGNAL(PrintStaticRoutes()),
                &routing_e_, SLOT(PrintStatic())
    );

    QObject::connect(
                &routing_e_, SIGNAL(PrintingDone()),
                &con_, SLOT(Restart())
    );

    QObject::connect(
                &con_, SIGNAL(AddStaticRoute(ForwardEntry)),
                &routing_e_, SLOT(AddStatic(ForwardEntry))
    );

    QObject::connect(
                &con_, SIGNAL(DelStaticRoute(int)),
                &routing_e_, SLOT(DelStatic(int))
    );

    arp_.setRE(&routing_e_);
    con_.start();
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ArpCink(const Traffic& t)
{
    std::cerr << t.in_intf_ << " recieved ARP\n";
}

void MainWindow::redrawArpTable(std::string intf)
{
    auto &table = intf_arp_view_mapping_[intf].tab();

    table.clearSelection();
    table.disconnect();
    table.clearContents();
    table.setRowCount(0);


    int index = 0 ;
    for (const auto & entry : arp_.GetTable(intf).mappings_){
        table.insertRow(index);
        table.setItem(index, 0, new QTableWidgetItem(entry.first.to_string().data()));
        table.setItem(index, 1, new QTableWidgetItem(entry.second.to_string().data()));
        index++;
    }
}

auto SwitchRouteOrigin(RouteSource s)
{
    switch(s){
    case CONN:
        return "C";
    case STATIC:
        return "S";
    case RIP:
        return "R";
    case IMPLICIT:
        return "I";
    }
    throw std::runtime_error("unhandled enum at SwitchRouteOrigin");
}

void MainWindow::redrawRouteTable()
{
    auto &table = *(ui -> routing_table);

    table.clearSelection();
    table.disconnect();
    table.clearContents();
    table.setRowCount(0);

    int index = 0 ;
    for (const auto & entry : routing_e_.GetTable()){
        table.insertRow(index);
        table.setItem(index, 0, new QTableWidgetItem(SwitchRouteOrigin(entry.second.origin_)));
        {
            std::stringstream tmp;
            tmp << int(entry.second.weight_);
            table.setItem(index, 1, new QTableWidgetItem(tmp.str().data()));
        }
        {
            std::stringstream tmp;
            tmp << entry.first.ip_.to_string() << '/' << int(entry.first.pref_l_);
            table.setItem(index, 2, new QTableWidgetItem(tmp.str().data()));
        }
        table.setItem(index, 3, new QTableWidgetItem(entry.second.exit_intf_.data()));
        table.setItem(index, 4, new QTableWidgetItem(entry.second.next_hop_.to_string().data()));
        index++;
    }



}
