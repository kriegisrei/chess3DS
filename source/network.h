#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include <fcntl.h>

#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <poll.h>
#include <sys/time.h>

#include <3ds.h>

#define CHESS_SERVER_ADDRESS "152.67.248.0"

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

static u32* SOC_buffer = NULL;

__attribute__((format(printf, 1, 2)))
void failExit(const char* fmt, ...);

s32 sock = -1;
u16 pingTimer = 0;

void networkShutdown() {
	if (sock > 0) { close(sock); }
	socExit();
}

void networkInit() {
	int ret;

	// allocate buffer for SOC service
	SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);

	if (SOC_buffer == NULL) {
		failExit("memalign: failed to allocate\n");
	}

	// Now intialise soc:u service
	if ((ret = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0) {
		failExit("socInit: 0x%08X\n", (unsigned int)ret);
	}

	// register socShutdown to run at exit
	// atexit functions execute in reverse order so this runs before gfxExit
	atexit(networkShutdown);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	if (sock < 0) {
		failExit("socket: %d %s\n", errno, strerror(errno));
	}

	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_addr.s_addr = inet_addr(CHESS_SERVER_ADDRESS);
	server.sin_family = AF_INET;
	server.sin_port = htons(8000);

	printf("Connecting to server. Wait 5 seconds.\n");

	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);

	if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) < 0) {
		if ((errno != EWOULDBLOCK) && (errno != EINPROGRESS)) {
			failExit("connect: %d %s\n", errno, strerror(errno));
		}
	}
}

void networkUpdate() {
	char recvBuffer[6];
	int bytes;

	pingTimer++;

	// Timeout of 5 seconds for connect.
	if (!networkState.connected) {
		struct pollfd event;
		event.fd = sock;
		event.events = POLLOUT;
			int ret = poll(&event, 1, 0);
			if (ret > 0 && event.revents & POLLOUT) {
				networkState.connected = true;
				char byte = 0x3D;
				if (send(sock, &byte, sizeof(char), 0) < 0) {
					failExit("send: %d %s\n", errno, strerror(errno));
				}

				consoleClear();
				printf("Connected, waiting for opponent.\n");
			}
		// Longer than 5 seconds
		if (pingTimer > 300) {
			failExit("timeout");
		}
		return;
	}

	// Check if socket connection still up every 5 seconds
	if (pingTimer > 300) {
		pingTimer = 0;
		char pingBuffer[6] = { 0x10, 0, 0, 0, 0, 0 };
		if (send(sock, pingBuffer, 1, 0) < 0) {
			failExit("send: %d %s\n", errno, strerror(errno));
		}
	}

	while ((bytes = recv(sock, recvBuffer, 6, 0)) == -1 && errno == EINTR);
	if (bytes < 0) {
		if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
			failExit("recv: %d %s\n", errno, strerror(errno));
		}
	}
	else if (bytes > 0) {
		switch (recvBuffer[0]) {
		case 0x00:
			networkState.gameStarted = true;
			networkState.systemColor = (Color)recvBuffer[1];
			// Ensure player doesn't try to move if turn packet is delayed
			gameState.playerTurn = (Color)(!(bool)networkState.systemColor);

			consoleClear();
			printf("Game started. You are color %s.\n", ((bool)networkState.systemColor) ? "black" : "white");

			break;
		case 0x01:
			gameState.playerTurn = (Color)recvBuffer[1];
			break;
		case 0x02:
			Position begin, end;
			begin.column = recvBuffer[1];
			begin.row = recvBuffer[2];
			end.column = recvBuffer[3];
			end.row = recvBuffer[4];
			if (recvBuffer[5] != 0) {
				chessBoard[begin.column][begin.row].currentPiece = (Piece)recvBuffer[5];
			}
			movePiece(begin, end);
			break;
		case 0x03:
			switch (recvBuffer[1]) {
			case 0x00:
				// Error. Ideally shouldn't happen.
				failExit("Game over: error.\n");
				break;
			case 0x01:
				// Draw.
				printf("Game over: draw.\n");
				// Socket will close, throwing error, and making user exit.
				break;
			case 0x02:
				printf("Game over: %s won.\n", ((bool)recvBuffer) ? "black" : "white");
				break;
			}
			break;
		}
	}
}

void netSendMove(Position start, Position end, Piece promotion) {
	char sendBuffer[6];
	sendBuffer[0] = 0x02;
	sendBuffer[1] = start.column;
	sendBuffer[2] = start.row;
	sendBuffer[3] = end.column;
	sendBuffer[4] = end.row;
	if (promotion == none) {
		if (send(sock, sendBuffer, 5, 0) < 0) {
			failExit("send: %d %s\n", errno, strerror(errno));
		}
	}
	else {
		sendBuffer[5] = promotion;
		if (send(sock, sendBuffer, 6, 0) < 0) {
			failExit("send: %d %s\n", errno, strerror(errno));
		}
	}
}

void failExit(const char* fmt, ...) {
	//---------------------------------------------------------------------------------

	if (sock > 0) close(sock);

	va_list ap;

	printf(CONSOLE_RED);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf(CONSOLE_RESET);
	printf("\nPress B to exit\n");

	while (aptMainLoop()) {
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_B) exit(0);
	}
}