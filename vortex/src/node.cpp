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
            continue;
        }

        // is this a callback messsage?
        // if match the callback to the reply it (sent msg_id) and handle it
        if (msg.body.contains("in_reply_to")) {
            std::string reply_id = msg.body["in_reply_to"];
            std::function<concurrencpp::result<void>(const Message)> callback;

            {
                std::lock_guard<std::mutex> lock(rpc_callbacks_mutex);
                auto it = rpc_callbacks.find(reply_id);
                if (it != rpc_callbacks.end()) {
                    callback = it->second;
                    rpc_callbacks.erase(it);
                }
            }

            if (callback) {
                executor()->submit([task = callback(msg)]() mutable { task.get(); });
            }
        } else {
            // not a callback -> match and dispatch to handler in task thread
            auto it = handlers.find(type);
            if (it != handlers.end()) {
                auto handler = it->second;
                executor()->submit([task = handler(msg)]() mutable { task.get(); });
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
               std::function<concurrencpp::result<void>(const Message)> rpc_handler) {

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

    auto promise_ptr = std::make_shared<concurrencpp::result_promise<Message>>(std::move(promise));
    {
        std::lock_guard<std::mutex> lock(rpc_callbacks_mutex);
        rpc_callbacks.emplace(msg_id,
                              [promise_ptr](const Message msg) -> concurrencpp::result<void> {
                                  promise_ptr->set_result(msg);
                                  co_return;
                              });
    }

    json res_body = body;
    res_body["msg_id"] = msg_id;
    send(dest, res_body);

    return result;
}

std::string Node::generate_id() {
    int id = this->next_msg_id.fetch_add(1);
    std::string result = this->node_id + "_" + std::to_string(id);
    return result;
}

concurrencpp::result<void> Node::on_init(const Message msg) {
    co_return;
}

concurrencpp::result<void> Node::handle_init(const Message msg) {

    node_id = msg.body["node_id"];
    msg.body["node_ids"].get_to(neighbors);
    json body;
    body["type"] = "init_ok";
    reply(msg, body);

    co_await on_init(msg);

    co_return;
}

Message Node::parse_message(const std::string &input) {
    json parsed = json::parse(input);
    Message msg;
    from_json(parsed, msg);
    return msg;
}

void Node::register_handlers() {
    throw std::logic_error("register_handlers() must be overridden in derived class.");
}

} // namespace vortex