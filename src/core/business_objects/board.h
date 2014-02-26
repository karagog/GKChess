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
public:

    Board(){
        for(GINT32 i = 0; i < ColumnCount(); ++i){
            for(GINT32 j = 0; j < RowCount(); ++j){
                m_board[i][j] = Square(this, i, j);
            }
        }
    }

    Board(const Board &b){
        for(GINT32 i = 0; i < ColumnCount(); ++i){
            for(GINT32 j = 0; j < RowCount(); ++j){
                m_board[i][j] = b.m_board[i][j];
            }
        }
    }

    /** Returns the number of rows. */
    int RowCount() const{ return 8; }

    /** Returns the number of columns. */
    int ColumnCount() const{ return 8; }

    /** Removes all pieces from the board.
     *  \note This does not resize the board - The only way to do that would be to create a new board.
    */
    void Clear(){
        for(int i = 0; i < ColumnCount(); ++i){
            for(int j = 0; j < RowCount(); ++j){
                RemovePiece(i, j);
            }
        }
    }



    /** Initializes a new piece on the given square.  The board will take
        ownership of the piece's memory.
    */
    void InitPiece(Piece *p, GUINT32 column, GUINT32 row){
        m_board[column][row].SetPiece(p);
    }

    /** Moves the piece at the source to the given destination square.
     *  If the destination square is occupied, its piece will be replaced by
     *  the one from source. If source is an empty square, this function does nothing.
   
        \note This does NOT do any chess rules validation, so it will assume any move
        is legal.
    */
    void Move(Square &source, Square &dest){
        Piece *p = source.GetPiece();
        if(p)
            dest.SetPiece(p);
    }
    
    /** Removes any piece that is on the square. */
    void RemovePiece(GUINT32 column, GUINT32 row){
        Square &s(m_board[column][row]);
        if(s.GetPiece())
            s.SetPiece(0);
    }

    /** Returns the square at the given column and row. */
    Square const &GetSquare(int column, int row) const{ return m_board[column][row]; }


private:

    Square m_board[8][8];

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_BOARD_H
