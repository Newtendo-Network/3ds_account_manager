#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include <optional>
#include <vector>
#include <algorithm>
#include <string>
#include "types.h"

const char *NASType_toString(NASType type)
{
	switch (type)
	{
	case NASType::NAS_LIVE:
		return "prod";
	case NASType::NAS_TEST:
		return "test";
	case NASType::NAS_DEV:
		return "dev";
	default:
		return "????";
	}
}

const char NASEnvironment_toString(NASEnvironment env)
{
	if (env >= NASEnvironment::NAS_ENV_U)
	{
		return 'U';
	}

	static char envs[] = {'L', 'C', 'S', 'D', 'I', 'T', 'J', 'X'};
	return envs[(int)env];
}

void WaitForStartPress()
{
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		// Your code goes here
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}
}

void WaitForStartPressOrReset()
{
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		// Your code goes here
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		if (kDown & KEY_SELECT)
		{
			aptInit();
			APT_HardwareResetAsync();
			aptExit();
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	gfxInitDefault();

	PrintConsole topConsole, bottomConsole;
	consoleInit(GFX_TOP, &topConsole);
	consoleInit(GFX_BOTTOM, &bottomConsole);

	consoleSelect(&topConsole);
	printf(CONSOLE_CYAN);
	printf("Welcome to the Newtendo Account Manager\n");
	printf("\n");
	printf(CONSOLE_RESET);

	Result rc = frdaInit();
	if (R_FAILED(rc))
	{
		printf("frdaInit() failed.\n");
		printf("\n");
		printf("Press START to exit.\n");
		WaitForStartPress();

		gfxExit();
		return 0;
	}

	printf("FRD is initialized!\n");
	printf("\n");

	// Force logout
	{
		ndmuInit();

		printf("Suspending friends daemon with NDM: ...\n" ESCAPE_GO_BACK_1_LINE);
		for (int i = 0; i < 6; i++)
		{
			static const char *suspension = "...";

			NDMU_SuspendDaemons(NDM_DAEMON_MASK_FRIENDS);

			ESCAPE_MOVE_RIGHT(36)
			printf(ESCAPE_CLEAR_UNTIL_END);
			printf("%s\r\n" ESCAPE_GO_BACK_1_LINE, i == 5 ? "done!" : &suspension[2 - (i % 3)]);

			svcSleepThread(3UL * (1000 * 1000 * 100));
		}

		printf("\n");

		ndmuExit();
	}

	// Set correct client SDK and unload current account
	{
		// Upgrade FRD Client SDK version otherwise it returns 0xC960C4F6
		// if(m_CurrentClientSDK < 0x70000C8) {
		// 		return 0xC960C4F6;
		// }
		FRDA_SetClientSdkVersion(0x70000C8);
		FRDA_Save();
		rc = FRDA_UnloadLocalAccount();
		if (R_FAILED(rc) && rc != (Result)0xC8A0C4EE) // 0xC8A0C4EE -> No acount loaded
		{
			printf("FRDA_UnloadLocalAccount() failed.\n");
			printf("\n");
			printf("Press START to exit.\n");
			WaitForStartPress();

			frdaExit();
			gfxExit();
			return 0;
		}
	}

	std::vector<NASLocalAccount> nasAccountList;

	printf("\n");
	printf("Loading NASC accounts: 0 / 255\n" ESCAPE_GO_BACK_1_LINE);
	for (int i = 0; i <= 255; i++)
	{
		u8 id = (u8)i;

		Result rc = FRDA_LoadLocalAccount(id);
		ESCAPE_MOVE_RIGHT(23)
		printf(ESCAPE_CLEAR_UNTIL_END);
		printf("%d / 255\r\n" ESCAPE_GO_BACK_1_LINE, id);

		if (R_SUCCEEDED(rc))
		{
			NASType type;
			NASEnvironment env;
			u8 envNum;

			FRDA_GetServerTypes(&type, &env, &envNum);
			NASLocalAccount localAccount = {.accountId = id, .type = type, .env = env, .envNum = envNum};

			FriendKey key;
			FRDA_GetMyFriendKey(&key);
			localAccount.pid = key.principalId;

			nasAccountList.push_back(localAccount);
			FRDA_UnloadLocalAccount();
		}
	}

	FRDA_UnloadLocalAccount();
	svcSleepThread(500UL * (1000 * 1000));

	// Show NASC accounts on the bottom screen
	{
		consoleSelect(&bottomConsole);
		consoleClear();

		printf("NASC Account List\n");
		printf("\n");
		for (auto &nas : nasAccountList)
			printf("ID %03d: %4s %c%01d | PID %lu\n", nas.accountId, NASType_toString(nas.type), NASEnvironment_toString(nas.env), nas.envNum, nas.pid);

		printf("\n");

		// Vertical line
		{
			char buffer[256] = {0};
			int i;
			for (i = 0; i < bottomConsole.consoleWidth; i++)
				buffer[i] = '_';

			buffer[i] = 0;
			printf("%s\n", buffer);
		}
	}

	consoleSelect(&topConsole);
	consoleClear();

	u8 accountId = 1;
	NASType accountType = NAS_LIVE;
	NASEnvironment accountEnv = NAS_ENV_L;
	std::string serverName = "Nintendo";

	printf("Select your server:\n");
	printf("\n");
	printf("Press X to use" CONSOLE_BLUE " Newtendo.\n" CONSOLE_RESET);
	printf("Press Y to use" CONSOLE_MAGENTA " Pretendo.\n" CONSOLE_RESET);

	u32 kDown = 0;
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		// Your code goes here
		kDown = hidKeysDown();

		if (kDown & KEY_X)
		{
			serverName = "Newtendo";
			accountId = NEWTENDO_ACCOUNT_ID;
			accountType = NAS_DEV;
			accountEnv = NAS_ENV_L;
			break;
		}
		else if (kDown & KEY_Y)
		{
			serverName = "Pretendo";
			accountId = PRETENDO_ACCOUNT_ID;
			accountType = NAS_TEST;
			accountEnv = NAS_ENV_L;
			break;
		}
	}

	consoleClear();

	auto search = std::find_if(nasAccountList.begin(), nasAccountList.end(), [accountId, accountType, accountEnv](NASLocalAccount const &n)
							   { return (n.accountId == accountId) && (n.env == accountEnv) && (n.type == accountType); });

	std::optional<NASLocalAccount> existingAccount = std::nullopt;
	if (search != nasAccountList.end())
		existingAccount = *search;

	printf("Press START to exit the app.\n");
	printf("\n");
	if (existingAccount)
	{
		printf("Press A to load your %s account.\n", serverName.c_str());
		printf("Press B to load your Nintendo account.\n");
		printf("Press X to delete your %s account.\n", serverName.c_str());
	}
	else
	{
		printf("Press Y to create your %s account.\n", serverName.c_str());
	}

	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		// Your code goes here
		kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		if (existingAccount && ((kDown & KEY_A) || (kDown & KEY_B) || (kDown & KEY_X)))
			break;

		if (!existingAccount && (kDown & KEY_Y))
			break;
	}

	consoleClear();

	if (!(kDown & KEY_START))
	{
		if (existingAccount)
		{
			if (kDown & KEY_A)
				rc = FRDA_LoadLocalAccount(accountId);

			if (kDown & KEY_B)
				rc = FRDA_LoadLocalAccount(1);

			if (kDown & KEY_X)
				rc = FRDA_DeleteLocalAccount(accountId);
		}
		else
		{
			if (kDown & KEY_Y)
				rc = FRDA_CreateLocalAccount(accountId, accountType, accountEnv, 1);
		}

		if (R_FAILED(rc))
		{
			printf("Action failed (0x%08lx).\n", rc);
			printf("\n");
			printf("Press SELECT to restart your console.\n");
			printf("Press START to exit.\n");

			WaitForStartPressOrReset();
		}
		else
		{
			printf("Action was a success!\n" ESCAPE_GO_BACK_1_LINE);
			printf("\n");
			printf("Press START to exit.\n");
			WaitForStartPress();
		}
	}

	u8 id;
	rc = FRDA_GetMyLocalAccountId(&id);
	if (R_FAILED(rc))
		FRDA_LoadLocalAccount(1);

	frdaExit();
	gfxExit();
	return 0;
}
