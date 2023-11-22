#include "EchoServer.hpp"

EchoServer::EchoServer() {}

void EchoServer::run() {
    std::string line;
    while (std::getline(std::cin, line)) {
        json data = json::parse(line);

        fmt::print(stderr, "Received {}\n", data.dump());

        if (data["body"]["type"] == "init") {
            node_id = data["body"]["node_id"];
        }

        fmt::print(stderr, "Initialized node {}", node_id);
    }
}

// {
//     "body" : {
//         "msg_id" : 1,
//         "node_id" : "n1",
//         "node_ids" : ["n1"],
//         "type" : "init"
//     },
//              "dest" : "n1",
//                       "id" : 0,
//                       "src" : "c0"
// }