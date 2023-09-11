#include "menu_NASC.h"

bool IsNewtendoAccount(NASLocalAccount &ent)
{
    return (ent.accountId == NEWTENDO_ACCOUNT_ID) && (ent.env == NAS_ENV_L) && (ent.envNum == 1) && (ent.type == NAS_DEV);
}

void MenuNASC::SetAccounts(std::vector<NASLocalAccount> &accounts)
{

    this->m_LocalAccounts = accounts;
    for (auto &ent : this->m_LocalAccounts)
    {
        if ((ent.accountId == 1) && (ent.env == NAS_ENV_L) && (ent.envNum == 1))
        {
            if (ent.type == NAS_LIVE)
            {
                ExtendedNASLocalAccount account(ent, "Nintendo (prod)");
                this->m_Accounts.push_back(account);
            }
            else if (ent.type == NAS_DEV)
            {
                if (ent.type == NAS_LIVE)
                {
                    ExtendedNASLocalAccount account(ent, "Nintendo (dev)");
                    this->m_Accounts.push_back(account);
                }
            }
        }
        else if ((ent.accountId == 2) && (ent.env == NAS_ENV_L) && (ent.envNum == 1))
        {
            if (ent.type == NAS_TEST)
            {
                ExtendedNASLocalAccount account(ent, "Pretendo");
                this->m_Accounts.push_back(account);
            }
        }
        else if (IsNewtendoAccount(ent))
        {
            ExtendedNASLocalAccount account(ent, "Pretendo");
            this->m_Accounts.push_back(account);
        }
    }
}

void MenuNASC::Init()
{
    ExtendedNASLocalAccount *acc = nullptr;
    for (auto &ent : m_Accounts)
    {
        if (IsNewtendoAccount(ent))
        {
            acc = &ent;
        }
    }

    if (acc)
    {
        m_DisableDeletion = false;
    }
}

static const std::vector<std::string> sMenuOptions = {"Delete Newtendo account",
                                                      "Load Newtendo account",
                                                      "Create Newtendo account"};

void MenuNASC::Render(PrintConsole *top, PrintConsole *bottom)
{
    if (m_IsCreating || m_IsDeleting || m_IsLoading)
    {
        if (R_SUCCEEDED(m_LastRc))
        {
            printf("Exit with START.\n");
            printf("\n");
            printf("\n");
            printf("The action succeeded! You can now exit the program.\n");
        }
        else
        {
            printf("Return to previous page with B.\n");
            printf("Exit with START.\n");
            printf("\n");
            printf("\n");
            printf("The action failed.\n");
        }
    }
    else
    {
        consoleSelect(top);

        printf("Number of accounts: %d\n", this->m_Accounts.size());
        printf("\n");

        printf("| ID |  Type  | Env |");
        for (auto &ent : this->m_Accounts)
        {
            char buf[12];
            snprintf(buf, sizeof(buf), "%c%01d", NASEnvironment_toString(ent.env), ent.envNum);
            printf("| %2x | %6s | %3s |", ent.accountId, NASType_toString(ent.type), buf);
        }

        consoleSelect(bottom);
        printf("Select an option with A.\n");
        printf("Return to previous page with B.\n");
        printf("Exit with START.\n");
        printf("\n");
        printf("\n");

        for (size_t i = 0; i < sMenuOptions.size(); i++)
        {
            auto opt = sMenuOptions.at(i);
            bool isSelected = (m_Option == i);
            if (isSelected)
            {
                if (m_Option <= 1 && m_DisableDeletion)
                    printf(CONSOLE_WHITE "> %s (doesn't exist yet)\n" CONSOLE_RESET, opt.c_str());
                else
                    printf(CONSOLE_WHITE "> %s\n" CONSOLE_RESET, opt.c_str());
            }
            else
            {
                printf(CONSOLE_GRAY "  %s\n" CONSOLE_RESET, opt.c_str());
            }
        }
    }
}

void MenuNASC::Update(u32 keys)
{
    size_t targetOption = m_Option;
    if (keys == KEY_DOWN)
        targetOption++;

    if (keys == KEY_UP)
        targetOption--;

    if (targetOption >= 0 && targetOption < (sMenuOptions.size() - 1))
    {
        if (m_DisableDeletion && targetOption <= 1)
        {
            ;
        }
        else
        {
            m_Option = targetOption;
        }
    }

    if (keys == KEY_B)
    {
        if (m_IsDeleting && R_FAILED(m_LastRc))
        {
            m_IsDeleting = false;
        }

        if (m_IsLoading && R_FAILED(m_LastRc))
        {
            m_IsLoading = false;
        }

        if (m_IsCreating && R_FAILED(m_LastRc))
        {
            m_IsCreating = false;
        }
    }

    if (keys == KEY_A && (!m_IsCreating && !m_IsLoading && !m_IsDeleting))
    {

        if (m_Option == 0)
        {
            m_LastRc = FRDA_DeleteLocalAccount(NEWTENDO_ACCOUNT_ID);
            m_IsDeleting = true;
        }
        else if (m_Option == 1)
        {
            m_LastRc = FRDA_LoadLocalAccount(NEWTENDO_ACCOUNT_ID);
            m_IsCreating = true;
        }
        else if (m_Option == 2)
        {
            m_LastRc = FRDA_CreateLocalAccount(NEWTENDO_ACCOUNT_ID, NAS_DEV, NAS_ENV_L, 1);
            m_IsCreating = true;
        }
    }
}