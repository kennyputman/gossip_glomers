#include "kv_node.h"

// TODO: upgrade this to an enum
vortex::KVNode::KVNode(const std::string &service_id) : service_id(service_id) {
}

concurrencpp::result<json> vortex::KVNode::read(const std::string &key) {
    json body;
    body["type"] = "read";
    body["key"] = key;

    auto rpc_res = co_await sync_rpc(service_id, body);
    co_return rpc_res.body;
}

concurrencpp::result<void> vortex::KVNode::write(const std::string &key, const json &value) {
    json body;
    body["type"] = "write";
    body["key"] = key;
    body["value"] = value;

    co_await sync_rpc(service_id, body);
    co_return;
}

concurrencpp::result<void> vortex::KVNode::compare_and_swap(const std::string &key,
                                                            const json &from, const json &to,
                                                            bool create_if_not_exists) {
    json body;
    body["type"] = "cas";
    body["key"] = key;
    body["from"] = from;
    body["to"] = to;
    if (create_if_not_exists) {
        body["create_if_not_exists"] = true;
    }

    co_await sync_rpc(service_id, body);
    co_return;
}
