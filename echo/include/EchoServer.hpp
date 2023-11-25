

#pragma once

#include "fmt/core.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <optional>
#include <string>

using json = nlohmann::json;

struct Message {
    std::string src;
    std::string dest;
    json body;

    Message() = default;
};

void to_json(json &j, const Message &m);
void from_json(const json &j, Message &m);

class EchoServer {
  public:
    EchoServer();
    virtual ~EchoServer() = default;

    void run();

  private:
    std::string node_id;
    int next_msg_id = 0;
};
