#include "fmt/core.h"
#include "maelstrom/message.hpp"
#include "maelstrom/node.hpp"

int main(int argc, char *argv[]) {
    auto node = std::make_shared<maelstrom::Node>();

    node->handle("echo", [&](maelstrom::Message msg) {
        maelstrom::Message response;
        response.src = msg.dest;
        response.dest = msg.src;
        response.body["type"] = "echo_ok";
        response.body["msg_id"] = node->get_next_msg_id();
        response.body["in_reply_to"] = msg.body["msg_id"];
        response.body["echo"] = msg.body["echo"];

        json j = response;
        fmt::print(stdout, "{}\n", j.dump());
    });

    node->run();
    return 0;
}