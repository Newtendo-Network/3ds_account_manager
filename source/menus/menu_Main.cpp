#include "menu_Main.h"

void MenuMain::Init()
{
}

static const std::vector<std::string> sMenuOptions = {"Manage friends (NASC)", "Manage accounts (NNAS)"};

void MenuMain::Render(PrintConsole *top, PrintConsole *bottom)
{

    consoleSelect(bottom);

    printf("Select an option with DPAD and A, press START to quit.\n");
    printf("\n");

    for (size_t i = 0; i < sMenuOptions.size(); i++)
    {
        auto opt = sMenuOptions.at(i);
        bool isSelected = (m_Option == i);
        if (isSelected)
        {
            printf(CONSOLE_WHITE "> %s\n" CONSOLE_RESET, opt.c_str());
        }
        else
        {
            printf(CONSOLE_GRAY "  %s\n" CONSOLE_RESET, opt.c_str());
        }
    }
}

void MenuMain::Update(u32 keys)
{
    size_t targetOption = m_Option;
    if (keys == KEY_DOWN)
        targetOption++;

    if (keys == KEY_UP)
        targetOption--;

    if (targetOption >= 0 && targetOption < (sMenuOptions.size() - 1))
        m_Option = targetOption;

    if (keys == KEY_A)
    {
        if (m_Option == 0)
            SetCurrentMenu(m_MenuNasc);
        else if (m_Option == 1)
            SetCurrentMenu(m_MenuNasc);
    }
}