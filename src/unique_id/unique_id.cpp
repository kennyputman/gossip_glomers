
#include "maelstrom/message.hpp"
#include "maelstrom/node.hpp"
#include <fmt/core.h>
#include <iostream>
#include <uuid.h>

int main(int argc, char const *argv[]) {
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size>{};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen{generator};

    auto node = std::make_shared<maelstrom::Node>();

    node->handle("generate", [&node, &gen](maelstrom::Message msg) {
        maelstrom::Message response;
        response.src = msg.dest;
        response.dest = msg.src;
        response.body["type"] = "generate_ok";
        response.body["msg_id"] = node->get_next_msg_id();
        response.body["in_reply_to"] = msg.body["msg_id"];

        uuids::uuid const id = gen();
        std::string uuid_str = uuids::to_string(id);

        response.body["id"] = uuid_str;

        json j = response;
        fmt::print(stdout, "{}\n", j.dump());
    });

    node->run();
    return 0;
}
