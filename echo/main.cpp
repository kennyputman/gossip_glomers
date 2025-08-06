#include "message.h"
#include "node.h"

class Echo : public vortex::Node {
  public:
    Echo() {
        register_handlers();
    }

  protected:
    void register_handlers() override {
        add_handler("echo", this, &Echo::handle_echo);
    }

  private:
    concurrencpp::result<void> handle_echo(const vortex::Message msg) {
        vortex::json body;
        body["type"] = "echo_ok";
        body["echo"] = msg.body["echo"];
        reply(msg, body);
        co_return;
    }
};

int main() {
    Echo echo;
    echo.run();
    return 0;
}