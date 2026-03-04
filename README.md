*This project has been created as part of the 42 curriculum by Sconiat, Llarrey.*

# ft_irc

## Description

**ft_irc** is a simplified implementation of an Internet Relay Chat (IRC) server written in C++ (C++98 standard).

The goal of this project is to recreate the core behavior of an IRC server following the RFC specifications. The server allows multiple clients to connect via TCP, join channels, exchange private messages, manage channel modes, and interact using standard IRC commands.

This project focuses on:
- Socket programming
- Network protocols (TCP/IP)
- Multi-client handling
- IRC protocol logic
- C++98 best practices

The server is compatible with standard IRC clients such as:
- `irssi`
- `nc`

---

## Features

- Multiple client connections
- Join / Part channels
- Private messaging (PRIVMSG)
- Channel operators
- Channel modes:
  - `i` (invite-only)
  - `t` (topic restriction)
  - `k` (password protection)
  - `l` (user limit)
  - `o` (operator management)
- KICK command
- MODE command
- INVITE command
- Proper IRC numeric replies (RPL / ERR)

---

## Instructions

### Compilation

The project is compiled using:

```bash
make