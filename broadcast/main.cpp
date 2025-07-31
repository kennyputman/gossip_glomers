#include <unordered_set>

#include "message.h"
#include "node.h"

using nlohmann::json;

class BroadcastNode : public vortex::Node {
  public:
    BroadcastNode() : messages() { register_handlers(); }

  protected:
    void register_handlers() override {
        add_handler("broadcast", this, &BroadcastNode::handle_broadcast);
        add_handler("broadcast_ok", this, &BroadcastNode::handle_broadcast_ok);
        add_handler("read", this, &BroadcastNode::handle_read);
        add_handler("topology", this, &BroadcastNode::handle_topology);
    }

  private:
    std::unordered_set<int> messages;
    std::map<std::string, std::vector<std::string>> topology;
    std::string broker;

    void handle_broadcast(const vortex::Message &msg) {
        int message = msg.body["message"];
        // CHECK: need to use mutex if updated to threads

        if (!messages.contains(message)) {
            this->messages.insert(message);
            flood(msg);

            json body;
            body["type"] = "broadcast_ok";
            reply(msg, body);
        }
    }

    void handle_read(const vortex::Message &msg) {
        json body;
        body["type"] = "read_ok";
        // CHECK: need to use mutex if updated to threads
        body["messages"] = this->messages;
        reply(msg, body);
    }

    void handle_topology(const vortex::Message &msg) {
        msg.body["topology"].get_to(this->topology);

        // very simple broker election
        auto it = topology.begin();
        this->broker = it->first;

        json body;
        body["type"] = "topology_ok";
        reply(msg, body);
    }

    // does nothing for now ... handles messages response broadcast
    void handle_broadcast_ok(const vortex::Message &msg) {}

    void flood(const vortex::Message &msg) {
        for (const auto &neighbor : this->neighbors) {
            if (neighbor != this->node_id && neighbor != msg.src) {
                send(neighbor, msg.body);
            }
        }
    }
};

int main() {
    BroadcastNode node;
    node.run();
    return 0;
}