#include "EchoServer.hpp"

EchoServer::EchoServer() {}

void EchoServer::run() {
    std::string line;
    while (std::getline(std::cin, line)) {
        json data = json::parse(line);

        auto msg = data.template get<Message>();

        std::string type = msg.body["type"];
        if (type == "init") {
            node_id = msg.body["node_id"];
            ++next_msg_id;

            Message response;
            response.src = node_id;
            response.dest = msg.src;
            response.body["msg_id"] = next_msg_id;
            response.body["in_reply_to"] = msg.body["msg_id"];
            response.body["type"] = "init_ok";

            json j = response;
            fmt::print(stdout, "{}\n", j.dump());

        } else if (type == "echo") {
            ++next_msg_id;

            Message response;
            response.src = node_id;
            response.dest = msg.src;
            response.body["type"] = "echo_ok";
            response.body["msg_id"] = next_msg_id;
            response.body["in_reply_to"] = msg.body["msg_id"];
            response.body["echo"] = msg.body["echo"];

            json j = response;
            fmt::print(stdout, "{}\n", j.dump());
        }
    }
}

void to_json(json &j, const Message &m) {
    j = json{{"src", m.src}, {"dest", m.dest}, {"body", m.body}};
}

void from_json(const json &j, Message &m) {
    j.at("src").get_to(m.src);
    j.at("dest").get_to(m.dest);
    j.at("body").get_to(m.body);
}
