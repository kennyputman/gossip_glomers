#include "EchoServer.hpp"
#include "fmt/core.h"

int main(int argc, char *argv[]) {
    auto server = std::make_shared<EchoServer>();
    server->run();
    return 0;
}