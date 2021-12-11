#include <iostream>
#include <string>

#include "app.hpp"

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " </path/to/UART> <baud rate> <TCP server port>" << std::endl;
        return 1;
    }

    std::string path(argv[1]);
    unsigned int rate = std::stol(argv[2]);
    uint16_t port = std::stoi(argv[3]);

    h4net::App app(path, rate, port);
    return app.Run();
}
