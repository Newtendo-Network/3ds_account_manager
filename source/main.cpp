#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include <vector>
#include "frda.h"

#define ESCAPE_SKIP_1_LINE "\x1b[1E"
#define ESCAPE_GO_BACK_1_LINE "\x1b[1A"
#define ESCAPE_CLEAR_CURRENT_LINE "\x1b[2K"
#define ESCAPE_CLEAR_CURRENT_CHAR "\x1b[P"
#define ESCAPE_CLEAR_UNTIL_END "\x1b[K"

#define ESCAPE_MOVE_RIGHT(n) (printf("\x1b[%dC", n));
#define ESCAPE_MOVE_LEFT(n) (printf("\x1b[%dD", n));

struct NASLocalAccount
{
	u8 accountId;
	NASType type;
	NASEnvironment env;
	u8 envNum;
};

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
			// TODO: quit
			;
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

	printf("\n");
	printf("Number of NASC accounts: %d\n", validAccountIDs.size());

	consoleSelect(&bottomConsole);
	consoleClear();

	for (auto &ent : validAccountIDs)
	{
		printf("NASC account %d: %5s | %c%01d\n", ent.accountId, NASType_toString(ent.type), NASEnvironment_toString(ent.env), ent.envNum);
	}

	// Main loop
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

	frdaExit();
	gfxExit();
	return 0;
}
