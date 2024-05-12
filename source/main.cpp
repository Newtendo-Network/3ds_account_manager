#include "main.h"

void printCenteredY(PrintConsole *console, int y, const char *color, const char *fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);

	vsnprintf(buffer, sizeof(buffer), fmt, args);

	consoleSelect(console);

	int x = (50 - strlen(fmt)) / 2;
	printf("\033[%d;%dH%s%s\n", y, x, color, buffer);

	va_end(args);
}

static const char *suspension = "...";
static int suspensionCount = 1;

void endFriendsDaemon(PrintConsole *console)
{
	consoleSelect(console);
	consoleClear();

	ndmDaemonStatus status;
	Result rc = NDMU_QueryStatus(NDM_DAEMON_FRIENDS, &status);
	rc = NDMU_SuspendDaemons(NDM_DAEMON_MASK_FRIENDS);
	while (status != NDM_DAEMON_STATUS_SUSPENDED)
	{
		printCenteredY(console, 15, R_SUCCEEDED(rc) ? CONSOLE_CYAN : CONSOLE_RED, "Suspending friends daemon%s", &suspension[2 - ((suspensionCount / 10) % 3)]);
		rc = NDMU_QueryStatus(NDM_DAEMON_FRIENDS, &status);
		gspWaitForVBlank();

		suspensionCount++;
	}
}

#define CHECK_RC(expr, msg, ...)                                           \
	if (R_FAILED(expr))                                                    \
	{                                                                      \
		consoleSelect(&topConsole);                                        \
		consoleClear();                                                    \
                                                                           \
		printCenteredY(&topConsole, 14, CONSOLE_RED, msg, ##__VA_ARGS__);  \
		printCenteredY(&topConsole, 16, CONSOLE_RED, "Exiting in 5...\n"); \
                                                                           \
		gspWaitForVBlank();                                                \
		svcSleepThread(5'000'000'000);                                     \
		goto exit;                                                         \
	}

static NASType nascTypes[3] = {NAS_LIVE, NAS_TEST, NAS_DEV};
static const char *nascNames[3] = {NINTENDO_TEXT, PRETENDO_TEXT, NEWTENDO_TEXT};
static u8 nascIDs[3] = {NINTENDO_ACCOUNT_ID, PRETENDO_ACCOUNT_ID, NEWTENDO_ACCOUNT_ID};
static NetworkAccount accounts[3];
static u8 actCount = 0;

static int selection = 0;

int main(int argc, char *argv[])
{
	gfxInitDefault();

	PrintConsole topConsole, bottomConsole;
	consoleInit(GFX_TOP, &topConsole);
	consoleInit(GFX_BOTTOM, &bottomConsole);

	ndmuInit();
	hidInit();
	aptInit();
	frdaInit();
	actaInit();

	endFriendsDaemon(&topConsole);

	Result rc = FRDA_SetClientSdkVersion(0x70000C8);

	u8 currentLocalNascAccount = 0;
	FRDA_GetMyLocalAccountId(&currentLocalNascAccount);

	CHECK_RC(rc, "Failed to set client SDK version: %08x\n", rc);

	FRDA_Save();
	FRDA_UnloadLocalAccount();

	rc = ACTA_GetCommonInfo(&actCount, 1, ACT_COMMON_INFO_ACCOUNT_COUNT);
	CHECK_RC(rc, "Failed to get account count: %08x\n", rc);

	for (int i = 0; i < 3; i++)
	{
		rc = FRDA_LoadLocalAccount(nascIDs[i]);
		accounts[i].nasc_local_id = nascIDs[i];
		accounts[i].printText = nascNames[i];

		if (R_SUCCEEDED(rc))
		{
			accounts[i].nasc_exists = true;

			NASType type;
			NASEnvironment env;
			u8 envNumber;

			rc = FRDA_GetServerTypes(&type, &env, &envNumber);
			CHECK_RC(rc, "Failed to get FRD server types: %08x\n", rc);

			accounts[i].nasc_type = type;

			if (type == nascTypes[i])
				accounts[i].do_match_env_type = true;

			for (int i = 0; i < actCount; i++)
			{
				accounts[i].slot_id = i + 1;

				u8 act_friend_local_account;
				rc = ACTA_GetAccountInfo(i + 1, &act_friend_local_account, 1, ACT_ACCOUNT_INFO_FRIEND_LOCAL_ACCOUNT_ID);
				CHECK_RC(rc, "Failed to get friend local ID (%d): %08x\n", rc, i + 1);

				if (act_friend_local_account != nascIDs[i])
					break;

				accounts[i].act_exists = true;

				char nnid[17] = {0};
				rc = ACTA_GetAccountInfo(i + 1, (u8 *)nnid, sizeof(nnid), ACT_ACCOUNT_INFO_ASSIGNED_NNID);
				CHECK_RC(rc, "Failed to get NNID (%d): %08x\n", rc, i + 1);

				wchar_t miiName[11] = {0};
				rc = ACTA_GetAccountInfo(i + 1, (u8 *)miiName, sizeof(miiName), ACT_ACCOUNT_INFO_MII_NAME);
				CHECK_RC(rc, "Failed to get Mii name (%d): %08x\n", rc, i + 1);

				accounts[i].nnid = nnid;
				accounts[i].mii_name = miiName;

				rc = ACTA_GetAccountInfo(i + 1, (u8 *)&accounts[i].pid, 4, ACT_ACCOUNT_INFO_ASSIGNED_PRINCIPAL_ID);
				CHECK_RC(rc, "Failed to get PID (%d): %08x\n", rc, i + 1);
			}

			rc = FRDA_UnloadLocalAccount();
			CHECK_RC(rc, "Failed to unload local account: %08x\n", rc);
		}
	}

	// =====================================================================================

	consoleSelect(&topConsole);
	consoleClear();
	printCenteredY(&topConsole, 13, CONSOLE_CYAN, "Select an account to log in:\n");
	printCenteredY(&topConsole, 15, CONSOLE_WHITE, "Press A to select an option\n");
	printCenteredY(&topConsole, 17, CONSOLE_WHITE, "UP / DOWN to navigate\n");
	printCenteredY(&topConsole, 19, CONSOLE_WHITE, "START to exit the app\n");

	consoleSelect(&bottomConsole);

	u8 defaultSlotId;
	u8 currentSlotId;

	rc = ACTA_GetCommonInfo(&defaultSlotId, 1, ACT_COMMON_INFO_DEFAULT_SLOT_ID);
	CHECK_RC(rc, "Failed to get default slot ID: %08x\n", rc);

	rc = ACTA_GetCommonInfo(&currentSlotId, 1, ACT_COMMON_INFO_CURRENT_SLOT_ID);
	CHECK_RC(rc, "Failed to get current slot ID: %08x\n", rc);

	while (aptMainLoop())
	{
		consoleClear();
		printf("\n");
		printf("%c " NINTENDO_TEXT " | %s " CONSOLE_GREEN "%s" CONSOLE_RESET "\n", selection == 0 ? '>' : ' ', accounts[0].GetExistanceText(), accounts[0].GetDefaultText(currentSlotId));
		printf("%c " PRETENDO_TEXT " | %s " CONSOLE_GREEN "%s" CONSOLE_RESET "\n", selection == 1 ? '>' : ' ', accounts[1].GetExistanceText(), accounts[1].GetDefaultText(currentSlotId));
		printf("%c " NEWTENDO_TEXT " | %s " CONSOLE_GREEN "%s" CONSOLE_RESET "\n", selection == 2 ? '>' : ' ', accounts[2].GetExistanceText(), accounts[2].GetDefaultText(currentSlotId));

		gspWaitForVBlank();
		gfxSwapBuffers();

		hidScanInput();
		u32 kDown = hidKeysDown();
		if (kDown & KEY_A)
		{
			break;
		}
		else if (kDown & KEY_UP)
		{
			selection = (selection - 1) % 3;
		}
		else if (kDown & KEY_DOWN)
		{
			selection = (selection + 1) % 3;
		}
		else if (kDown & KEY_START)
		{
			selection = -1;
			break;
		}
	}

	// =====================================================================================

	if (selection >= 0)
	{
		NASType type;
		uint8_t nascAccountId;
		switch (selection)
		{
		case 0:
			type = NAS_LIVE;
			nascAccountId = NINTENDO_ACCOUNT_ID;
			break;
		case 1:
			type = NAS_TEST;
			nascAccountId = PRETENDO_ACCOUNT_ID;
			break;
		case 2:
			type = NAS_DEV;
			nascAccountId = NEWTENDO_ACCOUNT_ID;
			break;
		}

		endFriendsDaemon(&topConsole);

		// =====================================================================================

		NetworkAccount *account = &accounts[selection];
		consoleSelect(&bottomConsole);
		consoleClear();

		if (!account->nasc_exists)
		{
			CHECK_RC(FRDA_CreateLocalAccount(nascAccountId, type, NAS_ENV_L, 1), "Failed to create NASC account: %08x\n", rc);
		}

		// =====================================================================================
	}

exit:

	ndmDaemonStatus status;
	NDMU_QueryStatus(NDM_DAEMON_FRIENDS, &status);

	if (status == NDM_DAEMON_STATUS_SUSPENDED)
		NDMU_ResumeDaemons(NDM_DAEMON_MASK_FRIENDS);

	if (currentLocalNascAccount != 0)
		FRDA_LoadLocalAccount(currentLocalNascAccount);

	actaExit();
	frdaExit();
	aptExit();
	hidExit();
	ndmuExit();
	gfxExit();
	return 0;
}
