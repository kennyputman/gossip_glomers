#include <chrono>
#include <random>
#include <stop_token>
#include <thread>
#include <unordered_set>

#include "message.h"
#include "seq_kv_node.h"
#include <iostream>

using nlohmann::json;

class CRDTNode : public vortex::SeqKVNode {

  public:
    CRDTNode() {
        register_handlers();
    }

  protected:
    void register_handlers() override {
        add_handler("init", this, &CRDTNode::handle_init);
        add_handler("add", this, &CRDTNode::handle_add);
        add_handler("read", this, &CRDTNode::handle_read);
    }

    concurrencpp::result<void> handle_init(const vortex::Message &msg) override {
        auto local_msg = msg;

        this->node_id = local_msg.body["node_id"];
        local_msg.body["node_ids"].get_to(this->neighbors);
        json body;
        body["type"] = "init_ok";
        reply(local_msg, body);

        co_await write(node_id, 0);
        co_return;
    }

  private:
    concurrencpp::result<void> handle_add(const vortex::Message &msg) {
        auto local_msg = msg;
        std::cerr << "START ADD [" << &msg << "] " << msg << std::endl;

        int delta = local_msg.body["delta"];

        auto res = co_await read(node_id);
        int value = res["value"].get<int>();

        int inc = delta + value;
        co_await write(node_id, inc);

        json body;
        body["type"] = "add_ok";

        std::cerr << "END ADD [" << &msg << "] " << msg << std::endl;
        reply(local_msg, body);
        co_return;
    }

    concurrencpp::result<void> handle_read(const vortex::Message &msg) {
        json body;
        body["type"] = "read_ok";
        body["value"] = 123;
        reply(msg, body);
        co_return;
    }
};

int main() {
    CRDTNode node;
    node.run();
    return 0;
}