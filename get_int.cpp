#include "get_int.hpp"


#include <string>
#include <tins/network_interface.h>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <fstream>


#include <iostream>

auto desired_interfaces()
{
    std::ifstream i("selected_interfaces.txt");
    if (!i){
        throw std::domain_error("Missing file 'selected_interfaces.txt' necessary configuration! ");
    }
    std::unordered_set<std::string> result;
    std::string line ;
    std::getline(i, line);
    while (!i.eof()){
        result.insert(line);
        std::getline(i, line);
    }

    return result;
}



std::vector<Tins::NetworkInterface> get_interfaces(){
    auto interfaces = Tins::NetworkInterface::all();
    auto desired = desired_interfaces();
    interfaces.erase(
        std::remove_if(
            interfaces.begin(),
            interfaces.end(),
            [&desired](const Tins::NetworkInterface& s){ return desired.find(s.name()) == desired.end();}
        ),
        interfaces.end()
    );
    if (interfaces.size() != 2){
        throw std::domain_error("Wrong interface config, didnt got desired interface count!");
    }
    return interfaces;
}
