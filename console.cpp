#include "console.hpp"
#include <iostream>
#include <tins/ip_address.h>
#include <string>
#include <cstring>
#include <vector>
#include "interfaceip.hpp"
#include <tins/exceptions.h>


auto tokenize(std::string x, const char* delim)
{
    auto input = x.data();
    std::vector<std::string> result;
    char *token = std::strtok(input, delim);
    while (token != nullptr) {
        result.push_back(token);
        token = std::strtok(nullptr, delim);
    }
    std::reverse(result.begin(), result.end());
    return result;
}

auto tokenize(std::string x)
{
    return tokenize(x, " ");
}

bool is_ip(const std::string& i)
{
    try {
        Tins::IPv4Address{i};
    } catch (Tins::invalid_address) {
        return false;
    }
    return true;
}
Console::Console(QObject *)
{
}


bool Console::parseDelRoute(std::string x)
{
    try {
        int index = std::stoi(x.data());
        emit DelStaticRoute(index);
        return true;
    } catch (const std::invalid_argument&) {

    } catch (const std::out_of_range&) {

    }
    std::cout << x << " is not a valid index!\n";
    return false;
}

void Console::parseAddRoute(const std::vector<std::string>& x)
{
    ForwardEntry route{IPInfo(), ExitInfo()};
    route.second.origin_ = STATIC;
    for (const auto& elem: x){
        auto kp = tokenize(elem,"=");
        if (kp.size() != 2 ){
            std::cout << elem << " is invalid keypair val for route\n";
            return;
        }
        if (kp.back() == "dest"){
            if (kp.front().find('/') != std::string::npos){
                auto ip = kp.front().substr(0, kp.front().find('/'));
                auto pref = kp.front().substr(kp.front().find('/')+1);
                try {
                    auto new_ip_ = Tins::IPv4Address(ip);
                    long new_pref_l = 0;
                    new_pref_l = std::stol(pref);
                    if (! is_network(new_ip_, Tins::IPv4Address::from_prefix_length(uint32_t(new_pref_l)))  ){
                        throw Tins::invalid_address();
                    }
                } catch (Tins::invalid_address) {
                    std::cerr << "cannot add route, invalid prefix!\n";
                    return;
                } catch (std::invalid_argument) {
                    std::cerr << "cannot add route, invalid prefix!\n";
                    return;
                }

                route.first = IPInfo(ip, pref); // err not handled
                continue;
            }
            std::cout << kp.front() << "\ninvalid prefix representation\n";
            return;
        }
        if (kp.back() == "dev"){
            route.second.exit_intf_ = kp.front();
            continue;
        }
        if (kp.back() == "next"){
            route.second.next_hop_ = kp.front();
            continue;
        }
        std::cout << kp.back() << "\n not a valid route definition\n";
        return;
    }
    emit AddStaticRoute(route);

}

Q_DECLARE_METATYPE(std::string)
void Console::run()
{
    std::string command ;
        qRegisterMetaType<std::string>("str");

    while (true){
        std::cout << ">> ";
        std::getline(std::cin, command);
        auto cmd = tokenize(command);
        if (cmd.size() == 0 ){
            continue;
        }
        if (cmd.back() == "arp"){
            cmd.pop_back();
            if (cmd.size() == 0){
                std::cout << command << "\nincomplete command";
                continue;
            }
            if (cmd.back() == "query"){
                cmd.pop_back();
                if (cmd.size() != 2){
                    std::cout << command << "\nincomplete command";
                    continue;
                }
                emit QueryArp(cmd[0], cmd[1]);
                continue;

            }
        }
        if (cmd.back() == "ip"){
            cmd.pop_back();
            if (cmd.size() == 0){
                std::cout << command << "\nincomplete command";
                continue;
            }
            if (cmd.back() == "addr"){
                cmd.pop_back();
                if (cmd.size() < 2){
                    std::cout << command << "\nincomplete command";
                    continue;
                }
                if (cmd.size() == 2 && cmd.front().find('/') != std::string::npos){
                    auto ip = cmd.front().substr(0, cmd.front().find('/'));
                    auto pref = cmd.front().substr(cmd.front().find('/')+1);
                    emit ChangeIP(cmd.back(), ip, pref);
                    continue;
                }
                if (cmd.size() == 3 && cmd.front().find('/') != std::string::npos){
                    emit ChangeIP(cmd[2], cmd[1], cmd[0].substr(cmd[0].find('/')+1));
                    continue;
                }
                std::cout << command << "\n invalid syntax after 6th token\n";
                continue;
            }
            if (cmd.back() == "route"){
                cmd.pop_back();
                if (cmd.size() == 0){
                    std::cout << command << "\nincomplete command";
                    continue;
                }
                if (cmd.back() == "add"){
                    cmd.pop_back();
                    parseAddRoute(cmd);
                    continue;
                }
                if (cmd.back() == "del"){
                    cmd.pop_back();
                    if (cmd.size() != 1){
                        std::cout << command << "\ndel missing index of route or gibberish at the end\n";
                        continue;
                    }
                    if (parseDelRoute(cmd.back())){
                        return;
                    }
                    continue;
                }
                if (cmd.back() == "show"){
                    if (cmd.size() != 1) {
                        std::cout << "gibberish after 'show' keyword present\n";
                        continue;
                    }
                    emit PrintStaticRoutes();
                    return;
                }
            }
        }
        std::cout << command << "\nCommand not found.\n";
    }
}

void Console::Restart()
{
    start();
}
