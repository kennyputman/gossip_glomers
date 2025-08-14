## Gossip Glomers
My solutions to Fly.io [Gossip Glomers Challenges](https://fly.io/dist-sys/)

### Challenges
- [X] Echo
- [X] Unique-Id Generation
- [X] Broadcast
  - [X] Single Node
  - [X] Multi-Node
  - [X] Fault Toleraint Multi-Node
  - [X] Efficient Multi-Node
- [X] Grow-Only Counter
- [ ] Kafka Style Log
  - [X] Single Node
  - [ ] Multi-Node
  - [ ] Efficient Multiple Node
- [ ] Totally Available
  - [ ] Single Node
  - [ ] Read Uncommitted
  - [ ] Read Committed

### Vortex Library
The Gossip Glomers challenges come with a Mealstrom/Go library that provides methods for `reply`, `send`, `rpc`, and `sync_rpc`. It also provides wrappers to interact with the [services](https://github.com/jepsen-io/maelstrom/blob/main/doc/services.md). Since my solution is in C++ I created these helpers in the `vortex` library. It includes a base `Node` class and an extension class `KVNode` that adds functionality for reading and writing to services such as `lin-kv`. 

### Dependencies
The project uses the [concurencpp](https://github.com/David-Haim/concurrencpp) library with C++ 20 coroutines. This could have been done using standard library concurrency tools such as threads, futures, etc... but I decided to use the `concurencpp` library as it included a nice executor framework for threadpooling. I eventually upgraded the solution to use coroutines as a way to become more familiar with them. 

### Challenges Overview
Here are some quick descriptions of what was done and the methods used to solve each challenge

#### Echo
Very simple. Just setup the `EchoNode` and created a single handler for `echo`. 

#### Unique-ID Generation
I added a  `generate_id` method for the `vortex::Node` class. It uses an `atomic<int`> plus the current nodes `node_id` to create a unique id. Maelstrom never recreates nodes so this method works. If they were a `uuid` would work too. 

#### Broadcast 
Here is where things started to get interesting. My current solutions is the <strong>simple_gossip</strong> node. I used the the list of neighbors given to the node at creation and implemented an epidemic gossiping protocol to reach consistency on the messages. This solution also handles network partitions since the gossiping protocol dumps the entire list of messages. This isn't the most efficient as it is retransmitting many previous messsages but it was still able to reach the efficiency benchmarks. 

#### Grow-Only
This one utilizes the `vortex::KVNode` base class to communicate with the `seq-kv` service provided by `maelstrom`. Each node handles `adds` to its own bucket in the `seq-kv` service using the `KVNode` helper methods. During reads the nodes fan out `local` messages to other nodes to get their current values and then returns the shared sum value. This relies on the `sync_rpc` method implemented in the `vortex::Node` class as  the nodes have to wait on the others to reply. 

// TODO: discuss partitions and timeouts


#### Kafka-Style-Log
current in progress.... 