#include <chrono>
#include <random>
#include <stop_token>
#include <thread>
#include <unordered_set>

#include "message.h"
#include "node.h"

using nlohmann::json;

class BroadcastNode : public vortex::Node {

  public:
    BroadcastNode() {
        register_handlers();
        gossip_thread = std::jthread([this](std::stop_token stoken) { gossip(stoken); });
    }

    ~BroadcastNode() override {
        gossip_thread.request_stop();
    }

  protected:
    void register_handlers() override {
        add_handler("broadcast", this, &BroadcastNode::handle_broadcast);
        add_handler("read", this, &BroadcastNode::handle_read);
        add_handler("topology", this, &BroadcastNode::handle_topology);
        add_handler("gossip", this, &BroadcastNode::handle_gossip);
    }

  private:
    std::unordered_set<int> messages;
    std::map<std::string, std::vector<std::string>> topology;
    std::mt19937 rng{std::random_device{}()};
    concurrencpp::async_lock lock;
    std::jthread gossip_thread;

    concurrencpp::result<void> handle_broadcast(const vortex::Message msg) {
        int message = msg.body["message"];

        {
            concurrencpp::scoped_async_lock raii_wrapper = co_await lock.lock(executor());
            messages.insert(message);
        }

        json body;
        body["type"] = "broadcast_ok";
        reply(msg, body);
        co_return;
    }

    concurrencpp::result<void> handle_read(const vortex::Message msg) {
        json body;
        body["type"] = "read_ok";
        {
            concurrencpp::scoped_async_lock raii_wrapper = co_await lock.lock(executor());
            body["messages"] = messages;
        }
        reply(msg, body);
        co_return;
    }

    concurrencpp::result<void> handle_topology(const vortex::Message msg) {
        msg.body["topology"].get_to(topology);

        json body;
        body["type"] = "topology_ok";
        reply(msg, body);
        co_return;
    }

    concurrencpp::result<void> handle_gossip(const vortex::Message msg) {
        std::unordered_set<int> incoming;
        msg.body["messages"].get_to(incoming);
        {
            concurrencpp::scoped_async_lock raii_wrapper = co_await lock.lock(executor());
            messages.insert(incoming.begin(), incoming.end());
        }
        co_return;
    }

    concurrencpp::result<void> gossip(std::stop_token stop_token) {
        while (!stop_token.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            json body;
            body["type"] = "gossip";
            {
                concurrencpp::scoped_async_lock raii_wrapper = co_await lock.lock(executor());
                body["messages"] = messages;
            }

            std::vector<std::string> shuffled(this->neighbors.begin(), this->neighbors.end());
            std::shuffle(shuffled.begin(), shuffled.end(), rng);

            int max_neighbors = 5;
            for (size_t i = 0; i < std::min<size_t>(max_neighbors, shuffled.size()); ++i) {
                if (shuffled[i] != this->node_id) {
                    send(shuffled[i], body);
                }
            }
        }
    }
};

int main() {
    BroadcastNode node;
    node.run();
    return 0;
}