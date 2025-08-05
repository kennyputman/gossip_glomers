#pragma once

#include "node.h"

using nlohmann::json;
namespace vortex {

class SeqKVNode : public Node {

  protected:
    concurrencpp::result<json> read(const std::string &key);
    concurrencpp::result<void> write(const std::string &key, const json &value);
    concurrencpp::result<void> cas(const std::string &key, const json &from, const json &to,
                                   bool create_if_not_exists);

  private:
    std::string service_id = "lin-kv";
};

} // namespace vortex
