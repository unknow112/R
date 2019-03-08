#include "console.hpp"
#include <iostream>
#include <tins/ip_address.h>
#include <string>
#include <sstream>
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
        if (std::string::npos != command.find("arp query")){
            auto rest = command.substr(10);
            std::cerr << "got '" << rest << "'\n";
            //Tins::IPv4Address ip{rest};
            emit QueryArp(rest, "enp0s25");
        }
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
