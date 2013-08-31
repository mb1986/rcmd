#include "ssh_executor.hpp"

#include <string>
#include <iostream>

int main(int argc, char const* argv[]) {

    std::string cmd = argv[0];
    std::cerr << "CMD: " << cmd << std::endl;

    SshExecutor ssh("192.168.1.202", 22, true);
    ssh.exec("jakaś komenda");

    return 0;
}

