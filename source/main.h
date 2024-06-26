#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include <optional>
#include <vector>
#include <algorithm>
#include <string>

#include "frda.h"
#include "acta.h"

#define ESCAPE_SKIP_1_LINE "\x1b[1E"
#define ESCAPE_GO_BACK_1_LINE "\x1b[1A"
#define ESCAPE_CLEAR_CURRENT_LINE "\x1b[2K"
#define ESCAPE_CLEAR_CURRENT_CHAR "\x1b[P"
#define ESCAPE_CLEAR_UNTIL_END "\x1b[K"

#define ESCAPE_MOVE_RIGHT(n) (printf("\x1b[%dC", n));
#define ESCAPE_MOVE_LEFT(n) (printf("\x1b[%dD", n));
#define CONSOLE_GRAY CONSOLE_ESC(90; 1m)

#define NINTENDO_TEXT CONSOLE_RED "Nintendo" CONSOLE_RESET
#define PRETENDO_TEXT CONSOLE_MAGENTA "Pretendo" CONSOLE_RESET
#define NEWTENDO_TEXT CONSOLE_CYAN "Newtendo" CONSOLE_RESET
#define UNKNOWN_TEXT CONSOLE_YELLOW "Unknown" CONSOLE_RESET

#define NINTENDO_ACCOUNT_ID 1
#define PRETENDO_ACCOUNT_ID 2
#define NEWTENDO_ACCOUNT_ID 77

struct NetworkAccount
{
    u8 slot_id = 0;
    std::string nnid;
    std::wstring mii_name;
    u32 pid = 0;

    NASType nasc_type = NAS_UNKNOWN;
    u8 nasc_local_id = 0;

    bool nasc_exists = false;
    bool act_exists = false;

    bool do_match_env_type = false;

    const char *printText = UNKNOWN_TEXT;

    const char *GetDefaultText(u8 defaultSlot)
    {

        if (slot_id == defaultSlot && defaultSlot)
            return "(default)";

        return "";
    }

    const char *GetExistanceText()
    {
        if (!nasc_exists && !act_exists)
            return "(none)     ";

        if (nasc_exists && !act_exists)
            return "(nasc)     ";

        if (!nasc_exists && act_exists)
            return "(act)      ";

        return "(nasc, act)";
    }
};
