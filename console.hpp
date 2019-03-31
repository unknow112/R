#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <QThread>
#include <string>
#include "re.hpp"
#include <tins/ip_address.h>
class Console : public QThread
{
    Q_OBJECT
public:
    explicit Console(QObject *parent = nullptr);
    void run();

signals:
    void QueryArp(const std::string&, const std::string&  );
    void ChangeIP(const std::string&, const std::string&, const std::string&);
    void PrintStaticRoutes();
    void AddStaticRoute(ForwardEntry);
    void DelStaticRoute(int);

public slots:
    void Restart();

private:
    bool parseDelRoute(std::string);
    void parseAddRoute(const std::vector<std::string>&);
};

#endif // CONSOLE_HPP
