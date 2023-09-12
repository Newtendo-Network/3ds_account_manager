#pragma once

#include "frda.h"

#define ESCAPE_SKIP_1_LINE "\x1b[1E"
#define ESCAPE_GO_BACK_1_LINE "\x1b[1A"
#define ESCAPE_CLEAR_CURRENT_LINE "\x1b[2K"
#define ESCAPE_CLEAR_CURRENT_CHAR "\x1b[P"
#define ESCAPE_CLEAR_UNTIL_END "\x1b[K"

#define ESCAPE_MOVE_RIGHT(n) (printf("\x1b[%dC", n));
#define ESCAPE_MOVE_LEFT(n) (printf("\x1b[%dD", n));
#define CONSOLE_GRAY CONSOLE_ESC(90; 1m)

#define PRETENDO_ACCOUNT_ID 2
#define NEWTENDO_ACCOUNT_ID 77

struct NASLocalAccount
{
    u32 pid;
    u8 accountId;
    NASType type;
    NASEnvironment env;
    u8 envNum;
};

const char *NASType_toString(NASType type);
const char NASEnvironment_toString(NASEnvironment env);