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

class GrowOnlyNode : public vortex::KVNode {

  public:
    GrowOnlyNode(const std::string service_id) : KVNode(service_id) {
        register_handlers();
    }

  protected:
    void register_handlers() override {
        add_handler("add", this, &GrowOnlyNode::handle_add);
        add_handler("read", this, &GrowOnlyNode::handle_read);
        add_handler("local", this, &GrowOnlyNode::handle_local);
    }

    concurrencpp::result<void> on_init(const vortex::Message msg) override {
        co_await write(node_id, 0);
        co_return;
    }

  private:
    std::vector<int> sum_values;
    concurrencpp::async_lock lock;

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
            concurrencpp::scoped_async_lock raii_wrapper = co_await lock.lock(executor());
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
            concurrencpp::scoped_async_lock raii_wrapper = co_await lock.lock(executor());
            sum_values.push_back(value);
        }

        co_return;
    }
};

int main() {
    GrowOnlyNode node("lin-kv");
    node.run();
    return 0;
}