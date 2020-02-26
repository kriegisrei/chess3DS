#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

int main(int argc, char* argv[])
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	printf("plac is not gae\n");
	printf("I don't like this\n");
	printf("Pretend this is chess, ok?\n");
	printf("Press START to exit. Press B for FRII GAM");
	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_B) {
			aptLaunchLibraryApplet(APPID_MINT, aptbuf, sizeof(aptbuf), 0);
		}
		else if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}

	gfxExit();
	return 0;
}
