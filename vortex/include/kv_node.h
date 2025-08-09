#pragma once

#include "node.h"

using nlohmann::json;
namespace vortex {

class KVNode : public Node {
  public:
    KVNode(const std::string &);

  protected:
    /**
     * @brief returns the entire body from a read using sync_rpc
     *
     * example {"type": "read_ok", "value": 1234}
     *
     * @param key
     * @return concurrencpp::result<json>
     */
    concurrencpp::result<json> read(const std::string &key);

    /**
     * @brief writes the key and value to the current nodes bucket
     *
     * @param key
     * @param value
     * @return concurrencpp::result<void>
     */
    concurrencpp::result<void> write(const std::string &key, const json &value);

    concurrencpp::result<void> compare_and_swap(const std::string &key, const json &from,
                                                const json &to, bool create_if_not_exists);

  private:
    std::string service_id = "Error: service_id not assigned";
};

} // namespace vortex
