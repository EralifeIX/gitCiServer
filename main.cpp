#include "webhooks/gitCIServer.h"
#include <iostream>

int main() {
    gitCIServer server(8888, "https://github.com/EralifeIX/gitCiServer", "/cloneDir");

    if (!server.start()) {
        std::cerr << "Failed to start CI server.\n";
        return 1;
    }

    std::cout << "GitCI Server started on port 8888. Press Enter to stop.\n";
    std::cin.get();

    server.stop();
    return 0;
}
