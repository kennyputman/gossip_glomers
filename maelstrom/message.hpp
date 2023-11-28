#pragma once

#include "nlohmann/json.hpp"
#include <string>

using json = nlohmann::json;

namespace maelstrom {

struct Message {
    std::string src;
    std::string dest;
    json body;

    Message() = default;
};

void to_json(json &j, const Message &m) {
    j = json{{"src", m.src}, {"dest", m.dest}, {"body", m.body}};
}

void from_json(const json &j, Message &m) {
    j.at("src").get_to(m.src);
    j.at("dest").get_to(m.dest);
    j.at("body").get_to(m.body);
}
} // namespace maelstrom