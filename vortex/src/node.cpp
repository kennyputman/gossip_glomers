#include "node.h"
#include <iostream>

namespace vortex {

Node::Node() : next_msg_id(0), node_id("error_node_id_not_init"), handlers() {
    add_handler("init", this, &Node::handle_init);
}

void Node::run() {
    std::string input;
    while (std::getline(std::cin, input)) {
        try {
            Message msg = parse_message(input);
            std::string type = msg.body["type"];
            handle_message(msg, type);

        } catch (const std::exception &e) {
            std::cerr << "Failed to parse json" << e.what() << std::endl;
        }
    }
}

void Node::reply(const Message &req, const json &body) {
    json res_body = body;
    res_body["in_reply_to"] = req.body["msg_id"];
    send(req.src, res_body);
}

void Node::send(const std::string &dest, const json &body) {
    json j;
    j["src"] = this->node_id;
    j["dest"] = dest;
    j["body"] = body;
    std::cout << j.dump() << "\n";
}

std::string Node::generate_id() {
    int id = this->next_msg_id.fetch_add(1);
    std::string result = this->node_id + "_" + std::to_string(id);
    return result;
}

void Node::handle_init(const Message &msg) {
    this->node_id = msg.body["node_id"];
    json body;
    body["type"] = "init_ok";
    reply(msg, body);
}

Message Node::parse_message(const std::string &input) {
    json parsed = json::parse(input);
    Message msg;
    from_json(parsed, msg);
    return msg;
}

void Node::handle_message(const Message &msg, const std::string &type) {
    auto it = handlers.find(type);
    if (it != handlers.end()) {
        it->second(msg);
    } else {
        std::cerr << "No handler registered for message type\n";
    }
}
} // namespace vortex