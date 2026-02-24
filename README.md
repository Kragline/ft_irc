*This project has been created as part of the 42 curriculum by armarake, nasargsy*

# Description
ft_irc project is the implementation of **Internet Relay Chat** or IRC, which is a text-based communication protocol on the Internet.
It offers real-time messaging that can be either public or private.
Users can exchange direct messages and join group channels.

In project we used RFC 1459 as standard and C/C++ for code base.

# Instructions
Below you can find instructions for building and using.

## Building
`make` - To build the app
`make clean` - To clean object files
`make fclean` To clean object files and executable binary file
`make re` - To execute `make fclean` and build the app

## Using
`./ircserv <port> <pass>` - To launch the app with the specified port number and password
E.g: `./ircsserv 5555 pass`

### NOTE
For ports from 0 to 1024 you need to run the app with SUID bit.

# Resources
https://datatracker.ietf.org/doc/html/rfc1459
https://cppreference.com/

