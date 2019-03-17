#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "get_int.hpp"
#include <tins/network_interface.h>
#include <iostream>
#include "traffic.hpp"
#include <vector>
#include <array>
#include <tins/pdu.h>
#include <string>
#include "outinterface.hpp"
#include "arp.hpp"
#include "interfaceip.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    auto interfaces  = get_interfaces();
    qRegisterMetaType<Traffic>("Traffic");

    arp_.setInterfaces(interfaces);
    for (const auto& intf: interfaces){
        input_intf_.emplace_back(intf);
    }

    for (auto & intf: input_intf_){
        QObject::connect(
                &intf, SIGNAL(ArpRecieved(const Traffic&)),
                &arp_, SLOT(processArp(const Traffic&))
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

//    QObject::connect(
//                &con_, SIGNAL(ChangeIP(const std::string&, const std::string&, const std::string&)),
//                &arp_, SLOT(setIP(const Tins::IPv4Address& , const std::string& , uint8_t ))
//    );

//    QObject::connect(
//                &con_, SIGNAL(ChangeIP(const std::string&, const std::string&, const std::string&)),
//                &arp_, SLOT(setIP(const Tins::IPv4Address& , const std::string& , uint8_t ))
//    );


    QObject::connect(
                &arp_,SIGNAL(ArpTableChanged(std::string)),
                this, SLOT(redrawArpTable(std::string))
    );
    QObject::connect(
                &arp_, SIGNAL(SendArpFrame(Traffic)),
                &output_intf_, SLOT(sendTraffic(Traffic))
    );
    QObject::connect(
                &con_, SIGNAL(QueryArp(const std::string&, const std::string&)),
                &arp_, SLOT(LookupIP(const std::string&, const std::string&))
    );

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

//void MainWindow::on_ip_change_button_clicked()
//{
//    auto new_ip = ui -> ip_inputer -> text().toStdString();
//    auto slash_pos = new_ip.find_first_of('/');
//    if (std::string::npos == slash_pos){
//        return;
//    }
//    emit IPChanged(new_ip.substr(0, slash_pos), "enp0s25", uint8_t(std::stoul(new_ip.substr(slash_pos+1))));
//}

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
