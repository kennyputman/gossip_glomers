#include "fmt/core.h"
#include "maelstrom/node.hpp"

int main(int argc, char *argv[]) {
    auto node = std::make_shared<maelstrom::Node>();
    node->run();
    return 0;
}