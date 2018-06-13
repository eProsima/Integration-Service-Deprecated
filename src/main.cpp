#include <iostream>
#include <memory>

#include "ISManager.h"


int main(int argc, char * argv[]){
    //Log::SetVerbosity(Log::Kind::Warning);
    //std::regex filter("RTCP(?!_SEQ)");  
    //Log::SetCategoryFilter(filter);
    if (!(argc > 1)){
        std::cout << "Usage: integration-services CONFIG_XML" << std::endl;
        return 0;
    }
    std::string path_to_config = argv[1];

    ISManager rs_manager(path_to_config);

    if (rs_manager.isActive()){
        std::cout << "\n### Integration Services is running, press any key for quit ###" << std::endl;
        fflush(stdout);
        std::cin.ignore();
    }
    else{
        std::cout << "Integration Services error: no active bridges" << std::endl;
    }
    return 0;
}
