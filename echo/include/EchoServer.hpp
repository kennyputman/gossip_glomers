

#pragma once

#include "fmt/core.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <string>

using json = nlohmann::json;

class EchoServer {
  public:
    EchoServer();
    virtual ~EchoServer() = default;

    void run();

  private:
    std::string node_id;
};
