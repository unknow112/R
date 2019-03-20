#include "console.hpp"
#include <iostream>
#include <tins/ip_address.h>
#include <string>
#include <cstring>
#include <vector>

auto tokenize(std::string x)
{
    auto input = x.data();
    std::vector<std::string> result;
    char *token = std::strtok(input, " ");
    while (token != nullptr) {
        result.push_back(token);
        token = std::strtok(nullptr, " ");
    }
    std::reverse(result.begin(), result.end());
    return result;
}

Console::Console(QObject *)
{
}

//Console::Console()
//{
//    std::cerr<<" ~~cink \n";
//    qRegisterMetaType<std::string>("str");
//}

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
        }
        std::cout << command << "\nCommand not found.\n";
    }
}
//void Console::run()
//{
//    std::string command ;
//    while (true){
//        std::getline(std::cin, command);
//        std::stringstream tokenise (command);
//        tokenise >> command;
//        if (command == "set"){
//            tokenise >> command;
//            if (command == "ip"){
//                std::string intf_name;
//                tokenise >> intf_name;

//            }
//        if (std::string::npos != command.find("set ip")){
//            command = command.substr(6);

//        }

//        // do stuff
//    }
//}
