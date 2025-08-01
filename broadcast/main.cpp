#include <chrono>
#include <random>
#include <thread>
#include <unordered_set>

#include "message.h"
#include "node.h"

using nlohmann::json;

class BroadcastNode : public vortex::Node {

  public:
    BroadcastNode() : messages(), topology() {
        register_handlers();
        gossip_thread = std::thread(&BroadcastNode::gossip, this);
    }

    ~BroadcastNode() override {
        stop_flag = true;
        if (gossip_thread.joinable())
            gossip_thread.join();
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
    std::mutex messages_mutex;
    std::atomic<bool> stop_flag{false};
    std::thread gossip_thread;
    std::mt19937 rng{std::random_device{}()};

    void handle_broadcast(const vortex::Message &msg) {
        int message = msg.body["message"];

        {
            std::lock_guard lock(messages_mutex);
            messages.insert(message);
        }

        json body;
        body["type"] = "broadcast_ok";
        reply(msg, body);
    }

    void handle_read(const vortex::Message &msg) {
        json body;
        body["type"] = "read_ok";
        {
            std::lock_guard lock(messages_mutex);
            body["messages"] = messages;
        }
        reply(msg, body);
    }

    void handle_topology(const vortex::Message &msg) {
        msg.body["topology"].get_to(topology);

        json body;
        body["type"] = "topology_ok";
        reply(msg, body);
    }

    void handle_gossip(const vortex::Message &msg) {
        std::unordered_set<int> incoming;
        msg.body["messages"].get_to(incoming);
        {
            std::lock_guard lock(messages_mutex);
            messages.insert(incoming.begin(), incoming.end());
        }
    }

    void gossip() {
        while (!stop_flag) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            json body;
            body["type"] = "gossip";
            {
                std::lock_guard lock(messages_mutex);
                body["messages"] = messages;
            }

            std::vector<std::string> shuffled(this->neighbors.begin(),
                                              this->neighbors.end());
            std::shuffle(shuffled.begin(), shuffled.end(), rng);

            int max_neighbors = 5;
            for (size_t i = 0;
                 i < std::min<size_t>(max_neighbors, shuffled.size()); ++i) {
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