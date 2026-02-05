#pragma once

#include "ClientState.hpp"
#include "ParseRequest.hpp"

#include "Server.hpp"

#include <sstream>

#define WHITE "\033[0m"
#define RED "\033[1;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define PURPLE "\033[0;35m"

#define CAP_LS "CAP * LS :\r\n"
#define JOIN_451 "451 JOIN :You have not registered\r\n"
#define JOIN_461 "461 JOIN :Not enough parameters\r\n"
