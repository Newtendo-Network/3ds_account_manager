#pragma once

#include <3ds/console.h>
#include <3ds/services/hid.h>

class Menu
{
public:
    enum class EMenuKind
    {
        MENU_MAIN = 0,
        MENU_NASC,
        MENU_NNAS,
        MENU_NASC_CREATION,
        MENU_NASC_DELETION,
        MENU_NNAS_CREATION,
        MENU_NNAS_DELETION
    };

public:
    Menu() = default;
    Menu(EMenuKind kind);

    EMenuKind kind;

    virtual void Init() = 0;
    virtual void Render(PrintConsole *top, PrintConsole *bottom) = 0;
    virtual void Update(u32 keys) = 0;
};