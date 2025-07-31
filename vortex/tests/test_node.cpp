#include "message.h"
#include "node.h"
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

using nlohmann::json;

class TestNode : public vortex::Node {
  public:
    TestNode() { register_handlers(); }

  protected:
    void register_handlers() override {}
};

TEST_CASE("Node handles init message") {
    json input_json = {{"src", "c1"},
                       {"dest", "n1"},
                       {"body",
                        {{"type", "init"},
                         {"msg_id", 1},
                         {"node_id", "n1"},
                         {"node_ids", {"n1", "n2", "n3"}}}}};

    std::string input_msg = input_json.dump();

    std::istringstream fake_input(input_msg + "\n");
    std::ostringstream fake_output;

    auto old_in = std::cin.rdbuf();
    auto old_out = std::cout.rdbuf();

    std::cin.rdbuf(fake_input.rdbuf());
    std::cout.rdbuf(fake_output.rdbuf());

    std::thread node_thread([] {
        TestNode node;
        node.run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cin.rdbuf(old_in);
    std::cout.rdbuf(old_out);

    node_thread.detach();

    std::string out = fake_output.str();
    auto json_out = nlohmann::json::parse(out);

    REQUIRE(json_out["src"] == "n1");
    REQUIRE(json_out["dest"] == "c1");
    REQUIRE(json_out["body"]["type"] == "init_ok");
    REQUIRE(json_out["body"]["in_reply_to"] == 1);
}
