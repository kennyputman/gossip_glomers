#include "node.h"
#include <iostream>
#include <syncstream>
#include <thread>

namespace vortex {

Node::Node() : next_msg_id(0), node_id("error_node_id_not_init"), handlers() {
    add_handler("init", this, &Node::handle_init);
}

void Node::run() {
    std::string input;
    while (std::getline(std::cin, input)) {
        // parse the message and type
        Message msg;
        std::string type;
        try {
            msg = parse_message(input);
            type = msg.body["type"];
        } catch (const std::exception &e) {
            std::osyncstream(std::cerr) << "Error parsing request" << input << std::endl;
        }

        // is this a callback messsage?
        // if match the callback to the reply it (sent msg_id) and handle it
        if (msg.body.contains("in_reply_to")) {
            std::string reply_id = msg.body["in_reply_to"];
            std::function<void(const Message &)> callback;

            {
                std::lock_guard<std::mutex> lock(rpc_callbacks_mutex);
                auto it = rpc_callbacks.find(reply_id);
                if (it != rpc_callbacks.end()) {
                    callback = it->second;
                    rpc_callbacks.erase(it);
                }
            }

            if (callback) {
                thread_pool_executor().submit([msg, callback]() { callback(msg); });
            }
        } else {
            // not a callback -> match and dispatch to handler in task thread
            auto it = handlers.find(type);
            if (it != handlers.end()) {
                auto handler = it->second;
                thread_pool_executor().submit([handler, msg] { handler(msg); });
            } else {
                std::string text = "Message type of: '" + type + "' is not registered";
                json body;
                body["type"] = "error";
                body["code"] = 10;
                body["text"] = text;
                reply(msg, body);
            }
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

    std::osyncstream(std::cout) << j.dump() << "\n";
    std::cout.flush();
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

concurrencpp::result<Message> Node::sync_rpc(const std::string &dest, const json &body) {

    std::string msg_id = generate_id();

    concurrencpp::result_promise<Message> promise;
    auto result = promise.get_result();

    {
        std::lock_guard<std::mutex> lock(rpc_callbacks_mutex);
        rpc_callbacks.emplace(msg_id, [promise = std::move(promise)](const Message &msg) mutable {
            promise.set_result(msg);
        });
    }

    json req_body = body;
    req_body["msg_id"] = msg_id;
    send(dest, req_body);

    return result;
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

} // namespace vortex