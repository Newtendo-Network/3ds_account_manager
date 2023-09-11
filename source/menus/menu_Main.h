#pragma once

#include "../menu.h"
#include "../types.h"

#include <vector>
#include <string>

#include "menu_NASC.h"

class MenuMain : public Menu
{

public:
    MenuMain() = default;

    void Init() override;
    void Render(PrintConsole *top, PrintConsole *bottom) override;
    void Update(u32 keys) override;

    size_t m_Option = 0;
    Menu *m_MenuNasc = new MenuNASC();
};