#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <QThread>
#include <string>
#include <tins/ip_address.h>
class Console : public QThread
{
    Q_OBJECT
public:
    explicit Console(QObject *parent = nullptr);
    void run();

signals:
    void QueryArp(const std::string&, const std::string&  );


public slots:
};

#endif // CONSOLE_HPP
