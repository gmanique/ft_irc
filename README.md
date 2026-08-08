*This project has been created as part of the 42 curriculum by gmanique, amados-s, lupan.*

# ft_irc

An implementation of a fully functional IRC (Internet Relay Chat) server written in C++ 98. This project allows multiple clients to connect simultaneously, chat in real-time, create channels, and perform channel administration using standard IRC protocols.

---

## Description

The main goal of **ft_irc** is to build a non-blocking, multi-client IRC server from scratch without using high-level network frameworks. It provides a deeper understanding of network socket programming, I/O multiplexing (`poll()`), memory management, and compliance with network protocols (RFC 1459 / RFC 2812).

### Key Features
* **I/O Multiplexing:** Non-blocking sockets managed with a single thread using `poll()`.
* **Client Authentication:** Password verification (`PASS`), nickname assignment (`NICK`), and user registration (`USER`).
* **IRCv3 Capability Negotiation:** Handles standard `CAP LS 302` requests for modern client compatibility.
* **Messaging:** Private direct messages (`PRIVMSG`) and channel broadcasting.
* **Channel Management:**
  * Joining (`JOIN`) and leaving (`PART`) channels.
  * Channel operator actions: kicking users (`KICK`), inviting users (`INVITE`), and setting topics (`TOPIC`).
* **Channel Modes (`MODE`):**
  * `i`: Set/remove Invite-only channel.
  * `t`: Set/remove Topic restrictions for channel operators.
  * `k`: Set/remove Channel key (password).
  * `o`: Give/take Channel operator privilege.
  * `l`: Set/remove User limit for channel.

---

## Instructions

### Prerequisites
* A C++ compiler supporting C++98 (`c++`).
* `make` utility.
* An IRC client such as **Irssi**.

### Compilation
To compile the server, run the following command at the root of the repository:

```bash
make
```

This will produce the ircserv executable.

### Execution

Run the server by providing a port number and a password:

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 mysecretpassword
```

### Connecting with Irssi

In another terminal, connect to your server using Irssi:

```bash
irssi
```
then
```
/connect 6667 mysecretpassword
```

### Technical Choices

- Language Standard: C++98, strictly complying with the project constraints (no C++11 standard libraries or modern features).

- Network Multiplexing: poll() was chosen to monitor network socket activities (reads and writes) without blocking the main execution thread.

- Command Parsing: Custom buffer system per client to handle partial commands and multiple commands sent in a single TCP packet separated by \r\n.

## Resources

### References & Documentation

- RFC 1459 - Internet Relay Chat Protocol

- RFC 2812 - Internet Relay Chat: Architecture & Client Protocol

- IRCv3 Specifications

- Irssi Documentation

- Beej's Guide to Network Programming

## AI Usage

Artificial Intelligence (Gemini) was used during this project for the following tasks:

- RFC Understanding & Protocol Debugging: Clarifying edge cases in IRC numerical responses, message formats (\r\n), and client-server negotiation steps (specifically CAP LS 302 flows with Irssi).

- Code Review & Edge Case Identification: Reviewing functions to spot non-blocking I/O pitfalls, memory leaks, iterator invalidations during map erasures, and signal handling (SIGPIPE).
