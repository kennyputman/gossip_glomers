#pragma once

#include <atomic>
#include <unordered_map>
#include <unordered_set>

#include "concurrencpp/concurrencpp.h"

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
     * @brief returns a thread pool executor for non blocking tasks
     *
     *  a general purpose executor that maintains a pool of threads. The thread pool executor is
     * suitable for short cpu-bound tasks that don't block.
     *
     * @return concurrencpp::thread_pool_executor&
     */
    concurrencpp::thread_pool_executor &thread_pool_executor() {
        return *runtime.thread_pool_executor();
    };

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
             std::function<concurrencpp::result<void>(Message)> rpc_handler);

    /**
     * @brief sends a synchronous rpc request ... returns the response message
     *
     * @param dest The destination node ID
     * @param body The JSON body of the message
     * @return concurrencpp::result<Message>
     *
     */
    concurrencpp::result<Message> sync_rpc(const std::string &dest, const json &body);

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
    void add_handler(const std::string &type, T *instance,
                     concurrencpp::result<void> (T::*method)(Message)) {
        handlers[type] = [instance, method](Message msg) -> concurrencpp::result<void> {
            co_return co_await (instance->*method)(std::move(msg));
        };
    }

    /**
     * @brief registers all handlers for class
     * Subclasses must override this to register all message handlers
     * using `add_handler()`. It should be called in the subclass constructor.
     */
    virtual void register_handlers();

    /**
     * @brief generates a unique id based on the node_id
     *
     * Will generate a unique id of structure [node_id]_[unique integer]
     *
     * example -> 'n1_123'
     * @return std::string
     */
    std::string generate_id();

    /**
     * @brief handles the initialization message
     *
     * If overridden the original initialization message handling must be preserved
     *
     * @param msg
     */
    virtual concurrencpp::result<void> handle_init(const Message msg);

  private:
    concurrencpp::runtime runtime;
    std::atomic<int> next_msg_id;
    std::unordered_map<std::string, std::function<concurrencpp::result<void>(Message)>> handlers;
    std::unordered_map<std::string, std::function<concurrencpp::result<void>(Message)>>
        rpc_callbacks;
    std::mutex rpc_callbacks_mutex;

    Message parse_message(const std::string &input);
};
} // namespace vortex