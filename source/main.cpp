#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include "Game.h"
#include "Input.h"
#include "Draw.h"

int main(int argc, char* argv[])
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	
	printf("Krieg shoud finish p4g\n");
	printf("also plac gae\n");
	printf("press start to exit, press anything else to get free game\n");

	drawInit();
	setupBoard();
	calculateAllMoves(white);

	// Main Loop
	while (aptMainLoop())
	{
		hidScanInput();
		u32 kDown = hidKeysDown();

		gameInput(kDown);
		drawUpdate();
	
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}

	drawFinish();
	gfxExit();
	return 0;
}
