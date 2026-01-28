#pragma once

#include "ClientState.hpp"

#define MAX_USERS 256
#define WELCOME_001 "001 <nick> :Welcome to the IRC Network <nick>\r\n\0"
#define WELCOME_002 "002 <nick> :Your host is server.name, running version 1.0\r\n\0"
#define WELCOME_003 "003 <nick> :This server was created today\r\n\0"
#define WELCOME_004 "004 <nick> server.name 1.0 o o\r\n\0"
#define JOIN_461 ":localhost 461 <nick> JOIN :Not enough parameters\r\n\0"
