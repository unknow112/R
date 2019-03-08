#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <tins/network_interface.h>
#include <tins/pdu.h>
#include <vector>
#include <string>
#include <tins/pdu.h>
#include "traffic.hpp"
#include "InInterface.hpp"
#include <QTableWidgetItem>
#include "arp.hpp"
#include "console.hpp"
#include "outinterface.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void IPChanged(const Tins::IPv4Address& , const std::string& , uint8_t );

public slots:
    void ArpCink(const Traffic&);
    void redrawArpTable();

private slots:
    void on_ip_change_button_clicked();



private:
    Ui::MainWindow *ui;
    std::vector<InInterface> input_intf_;
    OutInterface output_intf_;
    Arp arp_;
    std::vector<std::pair<QTableWidgetItem, QTableWidgetItem> > arp_view_;
    Console con_;
};

#endif // MAINWINDOW_HPP
