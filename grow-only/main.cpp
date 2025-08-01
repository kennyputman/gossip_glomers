#include <chrono>
#include <random>
#include <stop_token>
#include <thread>
#include <unordered_set>

#include "message.h"
#include "node.h"

using nlohmann::json;

class GrowOnlyNode : public vortex::Node {

  public:
    GrowOnlyNode() { register_handlers(); }

  protected:
    void register_handlers() override {
        add_handler("add", this, &GrowOnlyNode::handle_add);
        add_handler("read", this, &GrowOnlyNode::handle_read);
    }

  private:
    void handle_add(const vortex::Message &msg) {
        json body;
        body["type"] = "add_ok";
        reply(msg, body);
    }

    void handle_read(const vortex::Message &msg) {
        json body;
        body["type"] = "read_ok";
        body["value"] = 123;
        reply(msg, body);
    }
};

int main() {
    GrowOnlyNode node;
    node.run();
    return 0;
}