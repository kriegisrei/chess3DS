#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
int main(int argc, char* argv[])
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();
		u32 kDown = hidKeysDown();
		printf("Krieg shoud finish p4g\n");
		printf("also plac gae\n");
		printf("press start to exit, press anything else to get free game");
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}
	gfxExit();
	return 0;
}
