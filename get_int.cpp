#include "get_int.hpp"


#include <string>
#include <tins/network_interface.h>
#include <algorithm>
#include <vector>
#include <fstream>


std::vector<Tins::NetworkInterface> get_interfaces(){
    auto interfaces = Tins::NetworkInterface::all();
    interfaces.erase(
        std::remove_if(
            interfaces.begin(),
            interfaces.end(),
            [](const Tins::NetworkInterface& s){ return s.name() != "enp0s25";}
        ),
        interfaces.end()
    );
    return interfaces;
}
