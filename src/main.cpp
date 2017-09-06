#include <iostream>
#include <memory>

#include "RSManager.h"


int main(int argc, char * argv[]){

    if (!(argc > 1)){
        std::cout << "Usage: routing_service CONFIG_XML" << std::endl;
        return 0;
    }
    std::string path_to_config = argv[1];

    RSManager rs_manager(path_to_config);

    if (rs_manager.isActive()){
        std::cout << "\n### Routing Service is running, press any key for quit ###" << std::endl;
        fflush(stdout);
        std::cin.ignore();
    }
    else{
        std::cout << "Routing Service error: no active bridges" << std::endl;
    }
    return 0;
}
