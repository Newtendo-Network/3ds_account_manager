#pragma once

#include "../menu.h"
#include "../types.h"

#include <3ds/result.h>

#include <vector>
#include <string>

class MenuNASC : public Menu
{

    struct ExtendedNASLocalAccount : public NASLocalAccount
    {

        ExtendedNASLocalAccount(NASLocalAccount &account)
        {
            *this = account;
        }

        ExtendedNASLocalAccount(NASLocalAccount &account, std::string name)
        {
            *this = account;
            this->specialName = name;
        }

        std::string specialName;
    };

public:
    MenuNASC()
    {
        this->kind = EMenuKind::MENU_NASC;
    }

    MenuNASC(std::vector<NASLocalAccount> &accounts)
    {
        this->kind = EMenuKind::MENU_NASC;
        this->SetAccounts(accounts);
    }

    void Init() override;
    void Render(PrintConsole *top, PrintConsole *bottom) override;
    void Update(u32 keys) override;

    void SetAccounts(std::vector<NASLocalAccount> &accounts);

    std::vector<NASLocalAccount> m_LocalAccounts;
    std::vector<ExtendedNASLocalAccount> m_Accounts;
    size_t m_Option = 1;

    bool m_DisableDeletion = true;
    bool m_IsDeleting = false;
    bool m_IsLoading = false;
    bool m_IsCreating = false;
    Result m_LastRc = 0;
};