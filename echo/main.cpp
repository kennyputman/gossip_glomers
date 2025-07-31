#include "message.h"
#include "node.h"

class Echo : public vortex::Node {
  public:
    Echo() { register_handlers(); }

  protected:
    void register_handlers() override {
        add_handler(vortex::MessageType::Echo, this, &Echo::handle_echo);
    }

  private:
    void handle_echo(const vortex::Message &msg) {
        vortex::json body;
        body["type"] = "echo_ok";
        body["echo"] = msg.body["echo"];
        reply(msg, body);
    }
};

int main() {
    Echo echo;
    echo.run();
    return 0;
}