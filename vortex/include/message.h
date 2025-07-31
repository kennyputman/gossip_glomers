#pragma once

#include "nlohmann/json.hpp"
#include <string>

namespace vortex {
using json = nlohmann::json;

class Message {
  public:
    std::string src;
    std::string dest;
    json body;
};

inline void to_json(json &j, const Message &m) {
    j = json{{"src", m.src}, {"dest", m.dest}, {"body", m.body}};
}

inline void from_json(const json &j, Message &m) {
    j.at("src").get_to(m.src);
    j.at("dest").get_to(m.dest);
    j.at("body").get_to(m.body);
}
} // namespace vortex