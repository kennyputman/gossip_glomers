
# Gossip Glomers Challenge

This project includes the solutions to the [Gossip Glomers Challenge](https://fly.io/dist-sys/) by [fly.io](https://fly.io/) <br>

The Gossip Glomers Challenges are a series of distributed systems challenges build on top of [Maelstrom](https://github.com/jepsen-io/maelstrom) and [Jepsen](https://jepsen.io/)

## Library
The challenges come with a [Maelstrom Go Library](https://pkg.go.dev/github.com/jepsen-io/maelstrom/demo/go) that handles much of the boilerplate. I created my own C++ implementation as a library in the `/maelstrom` folder. 

## Solutions

- [x] Echo
- [ ] Unique ID Generation
- [ ] Broadcast
- [ ] Grow-Only Counter
- [ ] Kafka-Style Log
- [ ] Totally-Available

## Setting up the project

1. Install the Maelstrom prerequisites
    - The details can be found [here](https://github.com/jepsen-io/maelstrom/blob/main/doc/01-getting-ready/index.md)
    - You will need to install `openjdk-17-jdk`, `graphviz` and `gnuplot`
2. Bootstrap the project `chmod +x bootstrap.sh` and then `./bootstraph.sh`
   - This will download vcpkg and the project dependencies
3. Build the project `cmake --build build`

Dependencies
 - `fmtlib`
 - `nlohmann-json`