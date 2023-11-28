#pragma once

#include "fmt/core.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <optional>
#include <string>

#include "maelstrom/message.hpp"

using json = nlohmann::json;

namespace maelstrom {

class Node {
  public:
    Node() = default;
    ~Node() = default;

    void run() {
        std::string line;
        while (std::getline(std::cin, line)) {

            json data = json::parse(line);
            auto msg = data.template get<Message>();
            std::string type = msg.body["type"];

            ++next_msg_id;
            if (type == "init") {
                node_id = msg.body["node_id"];

                Message response;
                response.src = msg.dest;
                response.dest = msg.src;
                response.body["msg_id"] = next_msg_id;
                response.body["in_reply_to"] = msg.body["msg_id"];
                response.body["type"] = "init_ok";

                json j = response;
                fmt::print(stdout, "{}\n", j.dump());
            } else {
                if (handlers.find(type) != handlers.end()) {
                    handlers[type](msg);
                } else {
                    Message response;
                    response.src = node_id;
                    response.dest = msg.src;
                    response.body["type"] = "error";
                    response.body["msg_id"] = next_msg_id;
                    response.body["in_reply_to"] = msg.body["msg_id"];
                    response.body["code"] = 10;
                    response.body["text"] =
                        "unknown message type: [" + type + "]";

                    json j = response;
                    fmt::print(stdout, "{}\n", j.dump());
                }
            }
        }
    }

    void handle(std::string type, std::function<void(const Message)> handler) {
        handlers[type] = handler;
    }

    int get_next_msg_id() { return next_msg_id; }

  private:
    std::string node_id;
    int next_msg_id = 0;
    std::unordered_map<std::string, std::function<void(Message)>> handlers;
};

} // namespace maelstrom