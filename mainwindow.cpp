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

    output_intf_.setInterfaces(interfaces);

    for (auto & intf: input_intf_){
        QObject::connect(&intf, SIGNAL(ArpRecieved(const Traffic&)),
                &arp_, SLOT(processArp(const Traffic&)));
        intf.start();
    }

    QObject::connect(
                this, SIGNAL(IPChanged(const Tins::IPv4Address& , const std::string& , uint8_t )),
                &arp_, SLOT(setIP(const Tins::IPv4Address& , const std::string& , uint8_t ))
    );
    QObject::connect(
                &arp_,SIGNAL(ArpTableChanged()),
                this, SLOT(redrawArpTable())
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

void MainWindow::on_ip_change_button_clicked()
{
    auto new_ip = ui -> ip_inputer -> text().toStdString();
    auto slash_pos = new_ip.find_first_of('/');
    if (std::string::npos == slash_pos){
        return;
    }
    emit IPChanged(new_ip.substr(0, slash_pos), "enp0s25", std::stoul(new_ip.substr(slash_pos+1)));
}

void MainWindow::redrawArpTable()
{


    ui->arp_table_view->clearSelection();

    ui->arp_table_view->disconnect();

    ui->arp_table_view->clearContents();

    ui->arp_table_view->setRowCount(0);


    int index = 0 ;
    for (const auto & entry : arp_.fooGetTable().mappings_){
        ui -> arp_table_view -> insertRow(index);
        ui -> arp_table_view -> setItem(index, 0, new QTableWidgetItem(entry.first.to_string().data()));
        ui -> arp_table_view -> setItem(index, 1, new QTableWidgetItem(entry.second.to_string().data()));
        index++;
    }
}
