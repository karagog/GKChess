/*Copyright 2014 George Karagoulis

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef GKCHESS_BOARD_H
#define GKCHESS_BOARD_H

#include "gkchess_square.h"

NAMESPACE_GKCHESS;

class Piece;


/** Describes a chess board. */
class Board
{
    Square m_board[8][8];
public:

    Board();

    Board(const Board &);

    /** Returns the number of rows. */
    int RowCount() const{ return 8; }

    /** Returns the number of columns. */
    int ColumnCount() const{ return 8; }

    /** Removes all pieces from the board. */
    void Clear();



    /** Initializes a new piece on the given square.  The board will take
        ownership of the piece's memory.
    */
    void InitPiece(Piece *, int column, int row);

    /** Moves the piece at the source to the given destination square.
     *  If the destination square is occupied, its piece will be replaced by
     *  the one from source. If source is an empty square, this function does nothing.
   
        \note This does not do any chess rules validation, so it will assume any move
        is legal.
    */
    void Move(Square &source, Square &dest);
    
    /** Removes any piece that is on the square. */
    void RemovePiece(int column, int row);

    /** Returns the square at the given column and row. */
    Square const &GetSquare(int column, int row) const;
    /** Returns the square at the given column and row. */
    Square &GetSquare(int column, int row);

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_BOARD_H
