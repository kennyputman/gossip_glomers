#include "node.h"
#include <iostream>
#include <thread>

namespace vortex {

Node::Node() : next_msg_id(0), node_id("error_node_id_not_init"), handlers() {
    add_handler("init", this, &Node::handle_init);
}

void Node::run() {
    std::string input;
    while (std::getline(std::cin, input)) {
        handle_request(input);
    };
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

void Node::rpc(const std::string &dest, const json &body,
               std::function<void(const Message &)> rpc_handler) {

    std::string msg_id = generate_id();

    {
        std::lock_guard<std::mutex> lock(rpc_callbacks_mutex);
        rpc_callbacks.emplace(msg_id, rpc_handler);
    }

    json res_body = body;
    res_body["msg_id"] = msg_id;
    send(dest, res_body);
}

std::optional<Message> Node::sync_rpc(std::chrono::milliseconds timeout, const std::string &dest,
                                      const json &body) {
    return std::optional<Message>();
}

std::string Node::generate_id() {
    int id = this->next_msg_id.fetch_add(1);
    std::string result = this->node_id + "_" + std::to_string(id);
    return result;
}

void Node::handle_init(const Message &msg) {
    this->node_id = msg.body["node_id"];
    msg.body["node_ids"].get_to(this->neighbors);
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

void Node::handle_request(const std::string &input) {
    try {
        Message msg = parse_message(input);
        std::string type = msg.body["type"];
        handle_message(msg, type);
    } catch (const std::exception &e) {
        std::cerr << "Error parsing request" << input << std::endl;
    }
}

void Node::handle_message(const Message &msg, const std::string &type) {
    auto it = handlers.find(type);
    if (it != handlers.end()) {
        it->second(msg);
    } else {
        std::string text = "Message type of: '" + type + "' is not registered";
        json body;
        body["type"] = "error";
        body["code"] = 10;
        body["text"] = text;
        reply(msg, body);
    }
}
} // namespace vortex