#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include <vector>
#include "types.h"

#include "menus/menu_Main.h"

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

Menu *currentMenu = nullptr;
std::vector<Menu *> menuHistory;

void SetCurrentMenu(Menu *m)
{
	menuHistory.push_back(currentMenu);
	currentMenu = m;
}

void SetPreviousMenu()
{
	Menu *m = menuHistory.back();
	menuHistory.pop_back();
	SetCurrentMenu(m);
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
		printf("frdaInit() failed. Exiting in 3s ...\n" ESCAPE_GO_BACK_1_LINE);
		for (int i = 0; i < 3; i++)
		{

			ESCAPE_MOVE_RIGHT(30)
			printf(ESCAPE_CLEAR_UNTIL_END);
			printf("%ds ...\r\n" ESCAPE_GO_BACK_1_LINE, 3 - i);

			svcSleepThread(1 * (1000 * 1000 * 1000));
		}

		printf("\n");

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

			svcSleepThread(3 * (1000 * 1000 * 100));
		}

		printf("\n");

		ndmuExit();
	}

	// Set correct client SDK and unload accounts
	{
		// Upgrade FRD Client SDK version otherwise it returns 0xC960C4F6
		// if(m_CurrentClientSDK < 0x70000C8) {
		// 		return 0xC960C4F6;
		// }
		FRDA_SetClientSdkVersion(0x70000C8);
		rc = FRDA_UnloadLocalAccount();

		if (R_FAILED(rc))
		{
			printf("FRDA_UnloadLocalAccount() failed. Exit in 3 ...\n" ESCAPE_GO_BACK_1_LINE);
			for (int i = 0; i < 3; i++)
			{

				ESCAPE_MOVE_RIGHT(42)
				printf(ESCAPE_CLEAR_UNTIL_END);
				printf("%d ...\r\n" ESCAPE_GO_BACK_1_LINE, 3 - i);

				svcSleepThread(1 * (1000 * 1000 * 1000));
			}

			printf("\n");

			frdaExit();
			gfxExit();
			return 0;
		}
	}

	std::vector<NASLocalAccount> validAccountIDs;

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
			validAccountIDs.push_back(localAccount);
		}
	}

	svcSleepThread(1 * (1000 * 1000 * 1000));

	currentMenu = new MenuMain();

	// Main loop
	while (aptMainLoop())
	{

		consoleSelect(&topConsole);
		consoleClear();

		consoleSelect(&bottomConsole);
		consoleClear();

		currentMenu->Render(&topConsole, &bottomConsole);

		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		// Your code goes here
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		currentMenu->Update(kDown);
	}

	frdaExit();
	gfxExit();
	return 0;
}
