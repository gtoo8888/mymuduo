#include <iostream>

#include "Logger.h"

int main(int argc,char** argv){
    std::cout << "123" << std::endl;
    LOG_INFO("INFO");
    LOG_DEBUG("DEBUG");
    LOG_FATAL("FATAL");
    LOG_ERROR("ERROR");
    LOG_INFO("%s %d", "INFO", 234);
    return 0;
}