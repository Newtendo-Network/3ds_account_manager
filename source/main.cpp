#include "main.h"

void printCenter(PrintConsole *console, const char *color, const char *fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);

	int linecount = 0;
	for (int i = 0; fmt[i]; i++)
	{
		if (fmt[i] == '\n')
			linecount++;
	}

	vsnprintf(buffer, sizeof(buffer), fmt, args);

	consoleSelect(console);
	consoleClear();

	int length = strlen(fmt);
	int pos_x = (50 - length) / 2;
	int pos_y = (30 - linecount) / 2;

	printf("\033[%d;%dH%s%s\n", pos_y, pos_x, color, buffer);

	va_end(args);
}

void endFriendsDaemon(PrintConsole *console)
{
	ndmDaemonStatus status;
	NDMU_QueryStatus(NDM_DAEMON_FRIENDS, &status);
	NDMU_SuspendDaemons(NDM_DAEMON_MASK_FRIENDS);
	while (status != NDM_DAEMON_STATUS_SUSPENDED || suspensionCount < 60)
	{
		printCenter(console, CONSOLE_CYAN, "Suspending friends daemon%s", &suspension[2 - ((suspensionCount / 10) % 3)]);
		NDMU_QueryStatus(NDM_DAEMON_FRIENDS, &status);
		gspWaitForVBlank();

		suspensionCount++;
	}
}

static const char *suspension = "...";
static int suspensionCount = 1;

int main(int argc, char *argv[])
{
	gfxInitDefault();
	ndmuInit();
	hidInit();
	aptInit();
	frdaInit();
	actaInit();

	PrintConsole topConsole, bottomConsole;
	consoleInit(GFX_TOP, &topConsole);
	consoleInit(GFX_BOTTOM, &bottomConsole);

	endFriendsDaemon(&topConsole);

	// =====================================================================================

	int selection = 0;

	printCenter(&topConsole, CONSOLE_CYAN, "Select an account to log in (bottom screen):");

	consoleSelect(&bottomConsole);

	while (aptMainLoop())
	{

		consoleClear();
		printf("%c "
			   "Manage " NINTENDO_TEXT " network accounts"
			   "\n",
			   selection == 0 ? '>' : ' ');
		printf("%c "
			   "Manage " PRETENDO_TEXT " network accounts"
			   "\n",
			   selection == 1 ? '>' : ' ');
		printf("%c "
			   "Manage " NEWTENDO_TEXT " network accounts"
			   "\n",
			   selection == 2 ? '>' : ' ');

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
	}

	// =====================================================================================

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

	FRDA_UnloadLocalAccount();

	bool nascAccountExists = R_SUCCEEDED(FRDA_LoadLocalAccount(nascAccountId));

	NASType currentType;
	NASEnvironment currentEnv;
	uint8_t currentEnvNum;

	// TODO: ERROR HANDLING
	FRDA_GetServerTypes(&currentType, &currentEnv, &currentEnvNum);

	if (currentType != type)
	{
		// TODO: What to do when the current account is not the one we want to log in to?
	}

	// =====================================================================================

	actaExit();
	frdaExit();
	aptExit();
	hidExit();
	ndmuExit();
	gfxExit();
	return 0;
}
