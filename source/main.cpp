#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include "Game.h"
#include "Network.h"
#include "Input.h"
#include "Draw.h"

int main(int argc, char* argv[])
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	
	drawInit();
	setupBoard();
	calculateAllMoves(white);

	atexit(gfxExit);
	atexit(drawFinish);

	printf("Press left on the d-pad for single-system multiplayer. Press right for online multiplayer.\n");

	// Main Loop
	while (aptMainLoop())
	{
		hidScanInput();
		u32 kDown = hidKeysDown();

		if (!gamemode) {
			menuInput(kDown);
		}
		else {
			gameInput(kDown);
		}
		drawUpdate();
	
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}

	return 0;
}
