static touchPosition prevTouch = { 0, 0 };

static Position passIn;

void gameInput(u32 kDown) {
   touchPosition touch;

   //Read the touch screen coordinates
   hidTouchRead(&touch);

   // Handling pawn promotion selection
   if (gameState.promotion) {
      Piece replace = none;
      if (kDown & KEY_A) {
         replace = queen;
      }
      else if (kDown & KEY_B) {
         replace = bishop;
      }
      else if (kDown & KEY_X) {
         replace = rook;
      }
      else if (kDown & KEY_Y) {
         replace = knight;
      }
      if (replace) {
         chessBoard[gameState.selectedPiece.column][gameState.selectedPiece.row].currentPiece = replace;
         movePiece(gameState.selectedPiece, gameState.promotedPawnMove);
         gameState.pieceSelected = false;
         gameState.promotion = false;
      }
   }

   if (touch.px > 0 && touch.py > 0) {
      // Just got touched
      if (prevTouch.px == 0 && prevTouch.py == 0) {
         if (touch.px >= 40 && touch.px < 280) {
            if (!gameState.pieceSelected) {
               gameState.selectedPiece.column = (touch.px - 40) / 30;
               gameState.selectedPiece.row = 7 - touch.py / 30;
               // Make sure touched spot has a chess piece that the same color as current turn player
               if (chessBoard[gameState.selectedPiece.column][gameState.selectedPiece.row].currentPiece && chessBoard[gameState.selectedPiece.column][gameState.selectedPiece.row].pieceColor == gameState.playerTurn) {
                  gameState.pieceSelected = true;
               }
            }
            else {
               // Piece currently selected
               u8 touchColumn = (touch.px - 40) / 30;
               u8 touchRow = 7 - touch.py / 30;
               if (touchColumn == gameState.selectedPiece.column && touchRow == gameState.selectedPiece.row) {
                  // Piece touched. De-select it.
                  gameState.pieceSelected = false;
                  gameState.promotion = false;
               }
               else {
                  passIn.column = touchColumn;
                  passIn.row = touchRow;
                  if (validMove(gameState.selectedPiece, passIn)) {
                     // Prompt for wanted piece if pawn is being promoted.
                     if (chessBoard[gameState.selectedPiece.column][gameState.selectedPiece.row].currentPiece == pawn && (touchRow == 0 || touchRow == 7)) {
                        gameState.promotion = true;
                        printf("================================================\n");
                        printf("Press the following button to promote your pawn.\n");
                        printf("A: Queen\n");
                        printf("B: Bishop\n");
                        printf("X: Rook\n");
                        printf("Y: Knight\n");
                        printf("================================================\n");
                        gameState.promotedPawnMove = passIn;
                     }
                     else {
                        movePiece(gameState.selectedPiece, passIn);
                        gameState.pieceSelected = false;
                     }
                  }
               }
            }

         }
      }
   }
   
   prevTouch = touch;
}