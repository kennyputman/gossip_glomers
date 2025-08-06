#include <algorithm>
#include <chrono>
#include <random>
#include <set>
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
        add_handler("local", this, &CRDTNode::handle_local);
    }

    concurrencpp::result<void> handle_init(const vortex::Message msg) override {
        auto local_msg = msg;

        this->node_id = local_msg.body["node_id"];
        local_msg.body["node_ids"].get_to(this->neighbors);
        json body;
        body["type"] = "init_ok";
        reply(msg, body);

        co_await write(node_id, 0);
        co_return;
    }

  private:
    std::vector<int> sum_values;
    std::mutex sum_values_mutex;

    concurrencpp::result<void> handle_add(const vortex::Message msg) {

        int delta = msg.body["delta"];

        auto res = co_await read(node_id);
        int value = res["value"].get<int>();

        int inc = delta + value;
        co_await write(node_id, inc);

        json body;
        body["type"] = "add_ok";

        reply(msg, body);
        co_return;
    }

    concurrencpp::result<void> handle_local(const vortex::Message msg) {
        auto res = co_await read(node_id);
        int value = res["value"].get<int>();

        json body;
        body["type"] = "local_ok";
        body["value"] = value;
        reply(msg, body);
        co_return;
    }

    concurrencpp::result<void> handle_read(const vortex::Message msg) {

        auto res = co_await read(node_id);
        int value = res["value"].get<int>();

        {
            std::scoped_lock<std::mutex> lock(sum_values_mutex);
            sum_values.clear();
            sum_values.push_back(value);
        }

        std::vector<concurrencpp::result<void>> tasks;
        for (const std::string &id : neighbors) {
            if (id != node_id) {
                tasks.push_back(get_shared_sum_values({}, id));
            }
        }

        for (auto &task : tasks) {
            co_await task;
        }

        json body;
        body["type"] = "read_ok";
        body["value"] = std::accumulate(sum_values.begin(), sum_values.end(), 0);
        reply(msg, body);
        co_return;
    }

    concurrencpp::result<void> get_shared_sum_values(concurrencpp::executor_tag,
                                                     const std::string id) {
        json body;
        body["type"] = "local";
        body["msg_id"] = generate_id();

        auto res = co_await sync_rpc(id, body);

        int value = res.body["value"].get<int>();
        {
            std::scoped_lock<std::mutex> lock(sum_values_mutex);
            sum_values.push_back(value);
        }

        co_return;
    }
};

int main() {
    CRDTNode node;
    node.run();
    return 0;
}