#include <algorithm>
#include <chrono>
#include <random>
#include <set>
#include <stop_token>
#include <thread>
#include <unordered_set>

#include "kv_node.h"
#include "message.h"
#include <iostream>

using nlohmann::json;

struct Log {
    std::optional<size_t> committed;
    std::vector<int> data;

    size_t append(int msg) {
        data.push_back(msg);
        return data.size() - 1;
    }

    /*
        returns a list of lists for the key starting from the offset
    */
    std::vector<std::pair<size_t, int>> read(size_t offset) {
        std::vector<std::pair<size_t, int>> res;

        for (size_t i = offset; i < data.size(); i++) {
            res.push_back({i, data[i]});
        }

        return res;
    }
};

class SingleKafkaNode : public vortex::Node {

  public:
    SingleKafkaNode() {
        register_handlers();
    }

  protected:
    void register_handlers() override {
        add_handler("send", this, &SingleKafkaNode::handle_send);
        add_handler("poll", this, &SingleKafkaNode::handle_poll);
        add_handler("commit_offsets", this, &SingleKafkaNode::handle_commit_offsets);
        add_handler("list_committed_offsets", this,
                    &SingleKafkaNode::handle_list_committed_offsets);
    }

  private:
    std::unordered_map<std::string, Log> logs;
    std::unordered_map<std::string, int> commits;
    concurrencpp::async_lock lock;

    /*
        appends a message to the log identified by the key in `this->logs`
    */
    concurrencpp::result<void> handle_send(const vortex::Message msg) {
        int offset;
        {
            concurrencpp::scoped_async_lock guard = co_await lock.lock(executor());

            const std::string log_key = msg.body["key"];
            const int value = msg.body["msg"].get<int>();

            Log &log = logs[log_key];
            offset = log.append(value);
        }

        json body;
        body["type"] = "send_ok";
        body["offset"] = offset;

        reply(msg, body);
        co_return;
    }

    /*
        returns a map of logs with msgs starting from the offset
    */
    concurrencpp::result<void> handle_poll(const vortex::Message msg) {

        std::unordered_map<std::string, int> offsets;
        msg.body["offsets"].get_to(offsets);

        std::unordered_map<std::string, std::vector<std::pair<size_t, int>>> msgs;

        for (auto &[k, v] : offsets) {
            concurrencpp::scoped_async_lock guard = co_await lock.lock(executor());
            msgs[k] = logs[k].read(v);
        }

        json body;
        body["type"] = "poll_ok";
        body["msgs"] = msgs;

        reply(msg, body);
        co_return;
    }

    /*
        updates the commit log for the current log(key)
    */
    concurrencpp::result<void> handle_commit_offsets(const vortex::Message msg) {

        std::unordered_map<std::string, int> offsets;
        msg.body["offsets"].get_to(offsets);

        for (auto &[k, v] : offsets) {
            concurrencpp::scoped_async_lock guard = co_await lock.lock(executor());
            commits[k] = v;
        }

        json body;
        body["type"] = "commit_offsets_ok";

        reply(msg, body);
        co_return;
    }

    /*
        returns a map of logs and their current commited offset state
    */
    concurrencpp::result<void> handle_list_committed_offsets(const vortex::Message msg) {
        std::vector<std::string> keys;
        msg.body["keys"].get_to(keys);

        std::unordered_map<std::string, int> committed_offsets;
        {
            concurrencpp::scoped_async_lock guard = co_await lock.lock(executor());
            for (std::string &key : keys) {
                committed_offsets[key] = commits[key];
            }
        }

        json body;
        body["type"] = "list_committed_offsets_ok";
        body["offsets"] = committed_offsets;

        reply(msg, body);
        co_return;
    }
};

int main() {
    SingleKafkaNode node;
    node.run();
    return 0;
}