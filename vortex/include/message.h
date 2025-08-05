#pragma once

#include <string>
#include <syncstream>

#include "nlohmann/json.hpp"

namespace vortex {
using json = nlohmann::json;

class Message {
  public:
    std::string src;
    std::string dest;
    json body;
};

inline std::ostream &operator<<(std::ostream &os, const Message &m) {
    std::osyncstream sync_os(os);
    sync_os << "{ src: \"" << m.src << "\", dest: \"" << m.dest << "\", body: " << m.body.dump()
            << " }";
    return os;
}

inline void to_json(json &j, const Message &m) {
    j = json{{"src", m.src}, {"dest", m.dest}, {"body", m.body}};
}

inline void from_json(const json &j, Message &m) {
    j.at("src").get_to(m.src);
    j.at("dest").get_to(m.dest);
    j.at("body").get_to(m.body);
}
} // namespace vortex