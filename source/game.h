#include <vector>

struct Position {
   s8 column;
   s8 row;
};

enum Piece { none, king, queen, rook, knight, bishop, pawn };
enum Color { white, black };

struct BoardSquare {
   Piece currentPiece;
   Color pieceColor;
   bool pieceMoved = false;
};

struct GameState {
   Color playerTurn;
   Position kingPosWhite;
   Position kingPosBlack;
   bool pieceSelected;
   Position selectedPiece;
   bool check;
   Position prevMoveStart;
   Position prevMoveEnd;
   bool promotion;
   Position promotedPawnMove;
   int turns;
};

GameState gameState;

enum Gamemode { unselected, system_multiplayer, online_multiplayer};
Gamemode gamemode;

struct NetworkState {
   bool connected;
   bool gameStarted;
   Color systemColor;
};

NetworkState networkState;

// [columns][rows] / [x][y]
BoardSquare chessBoard[8][8];
std::vector<Position> possibleMoves[8][8];

void setupBoard() {
   const Piece LAYOUT[] = { rook, knight, bishop, queen, king, bishop, knight, rook };
   // First and last rows
   for (u8 i = 0; i < 8; i++) {
      chessBoard[i][0].currentPiece = LAYOUT[i];
      chessBoard[i][0].pieceColor = white;
      chessBoard[i][7].currentPiece = LAYOUT[i];
      chessBoard[i][7].pieceColor = black;
   }
   // Pawns
   for (u8 i = 0; i < 8; i++) {
      chessBoard[i][1].currentPiece = pawn;
      chessBoard[i][1].pieceColor = white;
      chessBoard[i][6].currentPiece = pawn;
      chessBoard[i][6].pieceColor = black;
   }
   gameState.playerTurn = white;
   gameState.kingPosWhite.column = 4;
   gameState.kingPosWhite.row = 0;
   gameState.kingPosBlack.column = 4;
   gameState.kingPosBlack.row = 7;
}

void calculatePieceMoves(BoardSquare (&chessBoard)[8][8], Position& position, std::vector<Position> (& moveList)) {
   BoardSquare & currentSquare = chessBoard[position.column][position.row];
   Position newMove;
   
   if (currentSquare.currentPiece == pawn) {
      // If white, moves up (row increases). If black, moves down (row decreases).
      s8 direction = 1;
      if (currentSquare.pieceColor == black) {
         direction = -1;
      }

      // Moving forward
      if (!chessBoard[position.column][position.row + direction].currentPiece) {
         newMove.column = position.column;
         newMove.row = position.row + direction;
         moveList.push_back(newMove);
         // Can go forward 2 steps?
         if ((!currentSquare.pieceMoved) && !chessBoard[position.column][position.row + 2 * direction].currentPiece) {
            newMove.row = position.row + 2 * direction;
            moveList.push_back(newMove);
         }
      }
      // Capturing left
      if (position.column > 0 && chessBoard[position.column - 1][position.row + direction].currentPiece) {
         if (chessBoard[position.column - 1][position.row + direction].pieceColor != currentSquare.pieceColor) {
            Position newMove;
            newMove.column = position.column - 1;
            newMove.row = position.row + direction;
            moveList.push_back(newMove);
         }
      }
      // Capturing right
      if (position.column < 7 && chessBoard[position.column + 1][position.row + direction].currentPiece) {
         if (chessBoard[position.column + 1][position.row + direction].pieceColor != currentSquare.pieceColor) {
            Position newMove;
            newMove.column = position.column + 1;
            newMove.row = position.row + direction;
            moveList.push_back(newMove);
         }
      }

      // En passant
      if (gameState.prevMoveEnd.row == position.row && chessBoard[gameState.prevMoveEnd.column][gameState.prevMoveEnd.row].currentPiece == pawn) {
         // Left or right
         if (gameState.prevMoveEnd.column == position.column - 1) {
            newMove.column = position.column - 1;
         }
         else if (gameState.prevMoveEnd.column == position.column + 1) {
            newMove.column = position.column + 1;
         }
         else {
            return;
         }
         // Opposing pawn is white.
         if (gameState.prevMoveStart.row - gameState.prevMoveEnd.row == -2) {
            newMove.row = position.row - 1;
            moveList.push_back(newMove);
         }
         // Opposing pawn is black.
         else if (gameState.prevMoveStart.row - gameState.prevMoveEnd.row == 2) {
            newMove.row = position.row + 1;
            moveList.push_back(newMove);
         }
         
      }

   }
   else if (currentSquare.currentPiece == knight) {
      // Right
      if (position.column + 2 < 8 && position.row + 1 < 8) {
         // Can only move onto other team's pieces.
         if (!chessBoard[position.column + 2][position.row + 1].currentPiece || chessBoard[position.column + 2][position.row + 1].pieceColor != currentSquare.pieceColor) {
            newMove.column = position.column + 2;
            newMove.row = position.row + 1;
            moveList.push_back(newMove);
         }
      }
      if (position.column + 2 < 8 && position.row - 1 >= 0) {
         if (!chessBoard[position.column + 2][position.row - 1].currentPiece || chessBoard[position.column + 2][position.row - 1].pieceColor != currentSquare.pieceColor) {
            newMove.column = position.column + 2;
            newMove.row = position.row - 1;
            moveList.push_back(newMove);
         }
      }
      // Left
      if (position.column - 2 >= 0 && position.row + 1 < 8) {
         if (!chessBoard[position.column - 2][position.row + 1].currentPiece || chessBoard[position.column - 2][position.row + 1].pieceColor != currentSquare.pieceColor) {
            newMove.column = position.column - 2;
            newMove.row = position.row + 1;
            moveList.push_back(newMove);
         }
      }
      if (position.column - 2 >= 0 && position.row - 1 >= 0) {
         if (!chessBoard[position.column - 2][position.row - 1].currentPiece || chessBoard[position.column - 2][position.row - 1].pieceColor != currentSquare.pieceColor) {
            newMove.column = position.column - 2;
            newMove.row = position.row - 1;
            moveList.push_back(newMove);
         }
      }
      // Up
      if (position.column + 1 < 8 && position.row + 2 < 8) {
         if (!chessBoard[position.column + 1][position.row + 2].currentPiece || chessBoard[position.column + 1][position.row + 2].pieceColor != currentSquare.pieceColor) {
            newMove.column = position.column + 1;
            newMove.row = position.row + 2;
            moveList.push_back(newMove);
         }
      }
      if (position.column - 1 >= 0 && position.row + 2 < 8) {
         if (!chessBoard[position.column - 1][position.row + 2].currentPiece || chessBoard[position.column - 1][position.row + 2].pieceColor != currentSquare.pieceColor) {
            newMove.column = position.column - 1;
            newMove.row = position.row + 2;
            moveList.push_back(newMove);
         }
      }
      // Down
      if (position.column + 1 < 8 && position.row - 2 >= 0) {
         if (!chessBoard[position.column + 1][position.row - 2].currentPiece || chessBoard[position.column + 1][position.row - 2].pieceColor != currentSquare.pieceColor) {
            newMove.column = position.column + 1;
            newMove.row = position.row - 2;
            moveList.push_back(newMove);
         }
      }
      if (position.column - 1 >= 0 && position.row - 2 >= 0) {
         if (!chessBoard[position.column - 1][position.row - 2].currentPiece || chessBoard[position.column - 1][position.row - 2].pieceColor != currentSquare.pieceColor) {
            newMove.column = position.column - 1;
            newMove.row = position.row - 2;
            moveList.push_back(newMove);
         }
      }
   }
   else if (currentSquare.currentPiece == king) {
      // Checks all places surrounding the king
      for (s8 i = position.column - 1; i <= position.column + 1; i++) {
         if (!(i >= 0 && i < 8)) {
            continue;
         }
         for (s8 j = position.row - 1; j <= position.row + 1; j++) {
            // Ensure king can't "move" to the spot it's currently on.
            if (!(j >= 0 && j < 8) || (i == position.column && j == position.row)) {
               continue;
            }
            // Can't move onto pieces of the same color
            if (!chessBoard[i][j].currentPiece || chessBoard[i][j].pieceColor != currentSquare.pieceColor) {
               newMove.column = i;
               newMove.row = j;
               moveList.push_back(newMove);
            }
         }
      }
      // Castling
      if (!currentSquare.pieceMoved) {
         // Left
         if (!chessBoard[0][position.row].pieceMoved && chessBoard[0][position.row].currentPiece == rook) {
            // Check if spaces are empty
            if (!chessBoard[1][position.row].currentPiece && !chessBoard[2][position.row].currentPiece && !chessBoard[3][position.row].currentPiece) {
               // Rook movement is handled later
               newMove.column = 2;
               newMove.row = position.row;
               moveList.push_back(newMove);
            }
         }
         // Right
         if (!chessBoard[7][position.row].pieceMoved && chessBoard[7][position.row].currentPiece == rook) {
            // Check if spaces are empty
            if (!chessBoard[5][position.row].currentPiece && !chessBoard[6][position.row].currentPiece) {
               // Rook movement is handled later
               newMove.column = 6;
               newMove.row = position.row;
               moveList.push_back(newMove);
            }
         }
      }
   } 
   else {
      if (currentSquare.currentPiece == rook || currentSquare.currentPiece == queen) {
         // Right
         for (s8 i = position.column + 1; i < 8; i++) {
            newMove.column = i;
            newMove.row = position.row;
            moveList.push_back(newMove);
            if (chessBoard[i][position.row].currentPiece) {
               // Can't move onto same color pieces.
               if (chessBoard[i][position.row].pieceColor == currentSquare.pieceColor) {
                  moveList.pop_back();
               }
               break;
            }
         }
         // Left
         for (s8 i = position.column - 1; i >= 0; i--) {
            newMove.column = i;
            newMove.row = position.row;
            moveList.push_back(newMove);
            if (chessBoard[i][position.row].currentPiece) {
               if (chessBoard[i][position.row].pieceColor == currentSquare.pieceColor) {
                  moveList.pop_back();
               }
               break;
            }
         }
         // Up
         for (s8 i = position.row + 1; i < 8; i++) {
            newMove.column = position.column;
            newMove.row = i;
            moveList.push_back(newMove);
            if (chessBoard[position.column][i].currentPiece) {
               if (chessBoard[position.column][i].pieceColor == currentSquare.pieceColor) {
                  moveList.pop_back();
               }
               break;
            }
         }
         // Down
         for (s8 i = position.row - 1; i >= 0; i--) {
            newMove.column = position.column;
            newMove.row = i;
            moveList.push_back(newMove);
            if (chessBoard[position.column][i].currentPiece) {
               if (chessBoard[position.column][i].pieceColor == currentSquare.pieceColor) {
                  moveList.pop_back();
               }
               break;
            }
         }
      }

      if (currentSquare.currentPiece == bishop || currentSquare.currentPiece == queen) {
         // Top right
         for (s8 i = position.column + 1, j = position.row + 1; i < 8 && j < 8; i++, j++) {
            newMove.column = i;
            newMove.row = j;
            moveList.push_back(newMove);
            if (chessBoard[i][j].currentPiece) {
               // Can't move onto same color pieces.
               if (chessBoard[i][j].pieceColor == currentSquare.pieceColor) {
                  moveList.pop_back();
               }
               break;
            }
         }
         // Top left
         for (s8 i = position.column - 1, j = position.row + 1; i >= 0 && j < 8; i--, j++) {
            newMove.column = i;
            newMove.row = j;
            moveList.push_back(newMove);
            if (chessBoard[i][j].currentPiece) {
               if (chessBoard[i][j].pieceColor == currentSquare.pieceColor) {
                  moveList.pop_back();
               }
               break;
            }
         }
         // Bottom right
         for (s8 i = position.column + 1, j = position.row - 1; i < 8 && j >= 0; i++, j--) {
            newMove.column = i;
            newMove.row = j;
            moveList.push_back(newMove);
            if (chessBoard[i][j].currentPiece) {
               if (chessBoard[i][j].pieceColor == currentSquare.pieceColor) {
                  moveList.pop_back();
               }
               break;
            }
         }
         // Bottom left
         for (s8 i = position.column - 1, j = position.row - 1; i >= 0 && j >= 0; i--, j--) {
            newMove.column = i;
            newMove.row = j;
            moveList.push_back(newMove);
            if (chessBoard[i][j].currentPiece) {
               if (chessBoard[i][j].pieceColor == currentSquare.pieceColor) {
                  moveList.pop_back();
               }
               break;
            }
         }
      }
   }
   
}

void handleSpecialMoves(BoardSquare (&activeBoard)[8][8], Position &kingPosition, Position &start, Position &end) {
   /* Handle Special Moves */

   // Account for king changing position
   if (activeBoard[start.column][start.row].currentPiece == king) {
      kingPosition = end;

      // Handle castling
      if (start.column - end.column == 2) {
         // To the left
         activeBoard[3][start.row] = activeBoard[0][start.row];
         activeBoard[3][start.row].pieceMoved = true;
         activeBoard[0][start.row].currentPiece = none;
      }
      else if (start.column - end.column == -2) {
         // To the right
         activeBoard[5][start.row] = activeBoard[7][start.row];
         activeBoard[5][start.row].pieceMoved = true;
         activeBoard[7][start.row].currentPiece = none;
      }

   }

   // En passant
   if (activeBoard[start.column][start.row].currentPiece == pawn) {
      // Pawn moves to the left or right
      if (start.column != end.column) {
         // If there wasn't a piece there previously, then pawn performed en passant. Remove appropriate pawn.
         if (!activeBoard[end.column][end.row].currentPiece) {
            activeBoard[end.column][start.row].currentPiece = none;
         }
      }
   }
}

void calculateAllMoves(Color playerColor) {
   // To be a possible move, a move needs to be within a pieces movement pattern, not blocked, and not result in an enemy piece being able to capture the king.
   BoardSquare tempBoard[8][8];
   Position passIn;
   // Go through each of the player's pieces and store their potential moves
   for (u8 i = 0; i < 8; i++) {
      for (u8 j = 0; j < 8; j++) {
         // Is there a piece there?
         if (chessBoard[i][j].currentPiece) {
            if (chessBoard[i][j].pieceColor == playerColor) {
               passIn.column = i;
               passIn.row = j;
               calculatePieceMoves(chessBoard, passIn, possibleMoves[i][j]);
            }
         }
      }
   }
   // Determine which moves aren't available because they expose the king.
   bool badMove;
   Position tempKingPos;
   for (u8 i = 0; i < 8 ; i++) {
      for (u8 j = 0; j < 8; j++) {

         tempKingPos = (playerColor == white) ? gameState.kingPosWhite : gameState.kingPosBlack;

         for (s8 k = 0; k < (s8)possibleMoves[i][j].size(); k++) {
            badMove = false;
            memcpy(tempBoard, chessBoard, sizeof(chessBoard));

            // Handle special moves
            passIn.column = i;
            passIn.row = j;
            handleSpecialMoves(tempBoard, tempKingPos, passIn, possibleMoves[i][j][k]);

            // Perform fake move
            tempBoard[possibleMoves[i][j][k].column][possibleMoves[i][j][k].row] = chessBoard[i][j];
            tempBoard[i][j].currentPiece = none;

            // Loop through every position in the resulting board. See if any piece is able to capture the king.
            for (u8 tempi = 0; tempi < 8; tempi++) {
               for (u8 tempj = 0; tempj < 8; tempj++) {

                  // Check moves only of pieces that exist and are the other team.
                  if (tempBoard[tempi][tempj].currentPiece && tempBoard[tempi][tempj].pieceColor != playerColor) {
                     std::vector<Position> tempMoves;
                     passIn.column = tempi;
                     passIn.row = tempj;
                     calculatePieceMoves(tempBoard, passIn, tempMoves);
                     // After calculating the moves for a piece, see if any of them result on the king.
                     for (size_t tempk = 0; tempk < tempMoves.size(); tempk++) {
                        if (tempMoves[tempk].column == tempKingPos.column && tempMoves[tempk].row == tempKingPos.row) {
                           badMove = true;
                           possibleMoves[i][j].erase(possibleMoves[i][j].begin() + k);
                           k--;
                           break;
                        } 
                     }
                  }
                  if (badMove) { break; }
               }
               if (badMove) { break; }
            }

         }

      }
   }

   // If there are no moves, it's a stalemate or checkmate.
   for (u8 i = 0; i < 8; i++) {
      for (u8 j = 0; j < 8; j++) {
         if (possibleMoves[i][j].size() > 0) {
            return;
         }
      }
   }

   // No moves.
   if (gameState.check) {
      printf("Checkmate %s won.\n", (!gameState.playerTurn) ? "black" : "white");
   }
   else {
      printf("Stalemate\n");
   }

}

// Returns if the attempted move is valid.
bool validMove(Position start, Position end) {
   for (u8 i = 0; i < possibleMoves[start.column][start.row].size(); i++) {
      if (end.column == possibleMoves[start.column][start.row][i].column && end.row == possibleMoves[start.column][start.row][i].row) {
         return true;
      }
   }
   return false;
}

// Check if validMove() beforehand.
void movePiece(Position start, Position end) {
   // Handle special moves
   handleSpecialMoves(chessBoard, ((gameState.playerTurn) ? gameState.kingPosBlack : gameState.kingPosWhite), start, end);

   // Perform move
   chessBoard[end.column][end.row] = chessBoard[start.column][start.row];
   chessBoard[end.column][end.row].pieceMoved = true;

   chessBoard[start.column][start.row].currentPiece = none;
   gameState.check = false;

   // Todo: track captures

   // See if the other king is checked
   std::vector<Position> findCheck;
   Position passIn;
   Position tempKingPos = (gameState.playerTurn == white) ? gameState.kingPosBlack : gameState.kingPosWhite;
   for (u8 x = 0; x < 8; x++) {
      for (u8 y = 0; y < 8; y++) {
         if (chessBoard[x][y].currentPiece && chessBoard[x][y].pieceColor == gameState.playerTurn) {
            passIn.column = x;
            passIn.row = y;
            calculatePieceMoves(chessBoard, passIn, findCheck);
            for (size_t z = 0; z < findCheck.size(); z++) {
               if (findCheck[z].column == tempKingPos.column && findCheck[z].row == tempKingPos.row) {
                  gameState.check = true;
                  break;
               }
            }
            findCheck.clear();
            if (gameState.check) { break; }
         }
      }
   }

   // Update previous move variables
   gameState.prevMoveStart = start;
   gameState.prevMoveEnd = end;
   gameState.turns++;
   gameState.playerTurn = (Color)!gameState.playerTurn;

   // Calculate all moves for next turn.
   for (u8 x = 0; x < 8; x++) {
      for (u8 y = 0; y < 8; y++) {
         possibleMoves[x][y].clear();
      }
   }
   calculateAllMoves(gameState.playerTurn);
}