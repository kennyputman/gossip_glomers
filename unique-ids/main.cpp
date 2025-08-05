#include "message.h"
#include "node.h"

class UniqueId : public vortex::Node {
  public:
    UniqueId() {
        register_handlers();
    }

  protected:
    void register_handlers() override {
        add_handler("generate", this, &UniqueId::handle_generate);
    }

  private:
    concurrencpp::result<void> handle_generate(const vortex::Message &msg) {
        vortex::json body;
        body["type"] = "generate_ok";
        body["id"] = this->generate_id();
        reply(msg, body);
        co_return;
    }
};

int main() {
    UniqueId unique_node;
    unique_node.run();
    return 0;
}