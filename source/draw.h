#include <citro2d.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

struct DrawObject {
   C3D_RenderTarget* bottom;
   u32 clrWhite;
   u32 clrBlack;
   u32 clrLightBrown;
   u32 clrDarkBrown;
   u32 clrGreen;
   u32 clrLightGreen;
   u32 clrDarkBlue;
};

DrawObject drawObject;

static C2D_SpriteSheet spriteSheet;
static C2D_Sprite sprites[12];

static void initSprites() {

   spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
   if (!spriteSheet) svcBreak(USERBREAK_PANIC);

   for (size_t i = 0; i < 12; i++)
   {
      C2D_Sprite* sprite = &sprites[i];

      C2D_SpriteFromSheet(sprite, spriteSheet, i);
      // Set center to bottom left corner of each sprite.
      C2D_SpriteSetCenter(sprite, 0.0f, 1.0f);
      C2D_SpriteSetScale(sprite, 0.5f, 0.5f);
   }
}


void drawInit() {
   romfsInit();
   C3D_Init(C3D_DEFAULT_CMDBUF_SIZE * 2);
   C2D_Init(C2D_DEFAULT_MAX_OBJECTS * 2);
   C2D_Prepare();

   drawObject.bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
   
   drawObject.clrWhite = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
   drawObject.clrBlack = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
   drawObject.clrLightBrown = C2D_Color32(0xD6, 0xB1, 0x8B, 0xFF);
   drawObject.clrDarkBrown = C2D_Color32(0xA5, 0x7A, 0x60, 0xFF);
   drawObject.clrGreen = C2D_Color32(0x75, 0x83, 0x54, 0xFF);
   drawObject.clrLightGreen = C2D_Color32(0xBE, 0xBA, 0x52, 0xFF);
   drawObject.clrDarkBlue = C2D_Color32(0x5C, 0x4C, 0x5B, 0xFF);
   
   initSprites();
}

void drawFinish() {
   C2D_SpriteSheetFree(spriteSheet);
   C2D_Fini();
   C3D_Fini();
}

void drawBoard() {
   bool currColor = true;
   for (u8 i = 0; i < 8; i++) {
      for (u8 j = 0; j < 8; j++) {
         C2D_DrawRectSolid(float(40 + i * 30), float(30 * j), 0.0f, 30.0f, 30.0f, ((currColor) ? drawObject.clrLightBrown : drawObject.clrDarkBrown));
         currColor = !currColor;
      }
      currColor = !currColor;
   }

}

void drawPieces() {
   u8 spriteNum;
   for (s8 i = 0; i < 8; i++) {
      for (s8 j = 0; j < 8; j++) {
         if (chessBoard[i][j].currentPiece) {
            spriteNum = (chessBoard[i][j].currentPiece - 1) * 2 + chessBoard[i][j].pieceColor;
            // Screen draws from top left, while chessBoard is from bottom left.
            C2D_SpriteSetPos(&sprites[spriteNum], float(40 + i * 30), float(240 - 30 * j));
            C2D_DrawSprite(&sprites[spriteNum]);
         }
      }
   }
}

void drawUpdate() {
   C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
   C2D_TargetClear(drawObject.bottom, drawObject.clrWhite);
   C2D_SceneBegin(drawObject.bottom);

   drawBoard();

   // Highlight the previous move.
   if (gameState.turns > 0) {
      C2D_DrawRectSolid(float(40 + gameState.prevMoveStart.column * 30), float(210 - 30 * gameState.prevMoveStart.row), 0.0f, 30.0f, 30.0f, drawObject.clrLightGreen);
      C2D_DrawRectSolid(float(40 + gameState.prevMoveEnd.column * 30), float(210 - 30 * gameState.prevMoveEnd.row), 0.0f, 30.0f, 30.0f, drawObject.clrLightGreen);
   }

   // If a piece is currently selected, highlight the spaces it can move to.
   if (gameState.pieceSelected) {      
      for (u8 i = 0; i < possibleMoves[gameState.selectedPiece.column][gameState.selectedPiece.row].size(); i++) {
         // Chessboard is from bottom left but screen draws from top left
         C2D_DrawRectSolid(float(40 + possibleMoves[gameState.selectedPiece.column][gameState.selectedPiece.row][i].column * 30), float(210 - 30 * possibleMoves[gameState.selectedPiece.column][gameState.selectedPiece.row][i].row), 0.0f, 30.0f, 30.0f, drawObject.clrGreen);
      }
      C2D_DrawRectSolid(float(40 + gameState.selectedPiece.column * 30), float(210 - 30 * gameState.selectedPiece.row), 0.0f, 30.0f, 30.0f, drawObject.clrDarkBlue);
   }

   drawPieces();

   C3D_FrameEnd(0);
}

