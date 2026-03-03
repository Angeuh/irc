*This project has been created as part of the 42 curriculum by Sconiat, Llarrey.*

# ft_irc

## Description

**ft_irc** is a simplified implementation of an Internet Relay Chat (IRC) server written in **C++ (C++98 standard)**.

The goal of this project is to recreate the core behavior of an IRC server following the RFC specifications. The server allows multiple clients to connect via TCP, join channels, exchange private messages, manage channel modes, and interact using standard IRC commands.

This project focuses on:

- Socket programming  
- Network protocols (TCP/IP)  
- Multi-client handling (non-blocking I/O with `epoll()`)  
- IRC protocol parsing and command handling  
- C++98 best practices  

The server is compatible with standard IRC clients such as:

- `irssi`
- `nc`

## Client Commands

### Registration

The following commands are required to properly register a client on the server:

- `/PASS <password>`  
  Authenticate with the server using the required connection password.

- `/NICK <nickname>`  
  Set or change your nickname. Must be unique on the server.

- `/USER <username> <hostname> <servername> :<realname>`  
  Register the user session by providing a username and real name.  
  This command completes the registration process when used with `PASS` and `NICK`.

### Channel Management

- `/JOIN <#channel>`  
  Join an existing channel or create it if it does not exist.

- `/JOIN <#channel> <password>`  
  Join a password-protected channel.

- `/PART <#channel>`  
  Leave a channel.

- `/MODE <#channel> <modes> [parameters]`  
  Manage channel modes ( i :invite-only, t : topic restriction, k : password, l : user limit, o : operator privileges).

- `/INVITE <nickname> <#channel>`  
  Invite a user to an invite-only channel.

- `/KICK <#channel> <nickname>`  
  Remove a user from a channel (operator only).

### Messaging

- `/PRIVMSG <nickname> :<message>`  
  Send a private message to a specific user.

- `/PRIVMSG <#channel> :<message>`  
  Send a message to all users in a channel.

(in irssi, /MSG is ok)

### Utility

- `/QUIT`  
  Disconnect from the server.

## Instruction

Make in project root, and launch server with ./ircserv <port> <password>

Then launch client like this :
- for irssi : irssi -n <nickname> -c localhost -p <port> -w <password> with same port and password as the server, and a nickname of your choice
- for netcat : nc -C localhost <port> (-C is necessary for proper RFC compliance)

## Ressources

### IRC Protocol Documentation :
- RFC 1459 — Internet Relay Chat Protocol : https://www.rfc-editor.org/rfc/rfc1459.html#section-4.2.8

### Network Programming References :

- IRSSI documentation : https://irssi.org/
- Article on server / client communication : http://chi.cs.uchicago.edu/chirc/irc_examples.html

### Linux manual pages:

- man socket
- man bind
- man listen
- man accept
- man epoll
- man recv
- man send
...

### C++ References :

- ISO C++98 Standard
- https://cppreference.com (C++98 documentation)

### Other :
- https://stackoverflow.com/

### AI utilisation :

Artificial Intelligence tools (ChatGPT) were used during the development of this project for:
- Clarifying parts of the IRC RFC specifications
- Understanding edge cases in IRC command handling
- Debugging socket behavior and epoll() usage

All implementation logic, architectural design, and final code were written and validated manually.
