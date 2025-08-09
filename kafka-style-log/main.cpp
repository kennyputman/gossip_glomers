#include <algorithm>
#include <chrono>
#include <random>
#include <set>
#include <stop_token>
#include <thread>
#include <unordered_set>

#include "kv_node.h"
#include "message.h"
#include <iostream>

using nlohmann::json;

class KafkaNode : public vortex::KVNode {

  public:
    KafkaNode(const std::string &id) : KVNode(id) {
        register_handlers();
    }

  protected:
    void register_handlers() override {
        add_handler("send", this, &KafkaNode::handle_send);
        add_handler("poll", this, &KafkaNode::handle_poll);
        add_handler("commit_offsets", this, &KafkaNode::handle_commit_offsets);
        add_handler("list_committed_offsets", this, &KafkaNode::handle_list_committed_offsets);
    }

  private:
    concurrencpp::result<void> handle_send(const vortex::Message msg) {

        json body;
        body["type"] = "send_ok";

        reply(msg, body);
        co_return;
    }

    concurrencpp::result<void> handle_poll(const vortex::Message msg) {

        co_return;
    }

    concurrencpp::result<void> handle_commit_offsets(const vortex::Message msg) {
        json body;
        body["type"] = "commit_offsets_ok";

        reply(msg, body);
        co_return;
    }

    concurrencpp::result<void> handle_list_committed_offsets(const vortex::Message msg) {
        json body;
        body["type"] = "list_committed_offsets_ok";

        reply(msg, body);
        co_return;
    }
};

int main() {
    KafkaNode node("seq-kv");
    node.run();
    return 0;
}