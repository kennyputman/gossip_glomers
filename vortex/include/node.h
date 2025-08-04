#pragma once

#include <atomic>
#include <unordered_map>
#include <unordered_set>

#include "message.h"

namespace vortex {
using nlohmann::json;

/**
 * Base class for all nodes in the system.
 * Handles initialization, receiving and the sending of messages
 *
 * Subclasses must implement the `register_handlers()` method to define
 * their specific message handling behavior. Each message type is mapped to
 * a handler function via `add_handler()`.
 */
class Node {
  public:
    Node();
    virtual ~Node() = default;

    void run();

  protected:
    std::unordered_set<std::string> neighbors;
    std::string node_id;

    /**
     * @brief Sends a reply to a given request message with the response body. Automatically fills
     * in the "in_reply_to" field.
     * @param req The incoming request message being replied to
     * @param body The JSON body to include in the reply
     */
    void reply(const Message &req, const json &body);

    /**
     * @brief Sends a message to the given destination node.
     * @param dest The destination node ID
     * @param body The JSON body of the message
     */
    void send(const std::string &dest, const json &body);

    /**
     * @brief sends an rpc request. The rpc_handler will be invoked when response message is
     * received
     *
     * @param dest The destination node ID
     * @param body The JSON body of the message
     */
    void rpc(const std::string &dest, const json &body,
             std::function<void(const Message &)> rpc_handler);

    /**
     * @brief
     *
     * @param timeout max time to wait for reply
     * @param dest The destination node ID
     * @param body The JSON body of the message
     * @return std::optional<Message>
     */
    std::optional<Message> sync_rpc(std::chrono::milliseconds timeout, const std::string &dest,
                                    const json &body);

    /**
     * @brief Registers a handler function for a specific message type.
     * @tparam T The type of the class (usually `this`)
     * @param type The MessageType to associate with the handler
     * @param instance The instance of the class containing the handler method
     * @param method The member function pointer to handle the message
     *
     *  Subclasses should call this function inside their overridden
     * `register_handlers()` method to bind each message type to a corresponding
     * handler member function.
     *
     * Example usage for derived node
     * ```c++
     * class Echo : public Node {
     *   public:
     *     Echo() {
     *         register_handlers();
     *     }
     *
     *   protected:
     *     void register_handlers() override {
     *         add_handler(MessageType::Echo, this, &Echo::handle_echo);
     *     }
     *
     *   private:
     *     void handle_echo(const Message& msg);
     * };
     * ```
     */
    template <typename T>
    void add_handler(const std::string &type, T *instance, void (T::*method)(const Message &msg)) {
        handlers[type] = [instance, method](const Message &msg) { (instance->*method)(msg); };
    }

    /**
     * @brief registers all handlers for class
     * Subclasses must override this to register all message handlers
     * using `add_handler()`. It should be called in the subclass constructor.
     */
    virtual void register_handlers() = 0;

    /**
     * @brief generates a unique id based on the node_id
     *
     * Will generate a unique id of structure [node_id]_[unique integer]
     *
     * example -> 'n1_123'
     * @return std::string
     */
    std::string generate_id();

  private:
    std::atomic<int> next_msg_id;
    std::unordered_map<std::string, std::function<void(const Message &)>> handlers;
    std::unordered_map<std::string, std::function<void(const Message &)>> rpc_callbacks;
    std::mutex rpc_callbacks_mutex;

    Message parse_message(const std::string &input);
    void handle_request(const std::string &input);
    void handle_message(const Message &msg, const std::string &type);
    void handle_init(const Message &msg);
};
} // namespace vortex