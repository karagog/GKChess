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

#ifndef GKCHESS_SQUARE_H
#define GKCHESS_SQUARE_H

#include "gkchess_globals.h"

NAMESPACE_GKCHESS;

class Board;
class Piece;


/** Describes one square of the chess board. */
class Square
{
    friend class Board;
    friend class GameLogic;

    Board *board;
    
    Square *north;
    Square *north_east;
    Square *east;
    Square *south_east;
    Square *south;
    Square *south_west;
    Square *west;
    Square *north_west;
    
    bool en_passant;
    bool castle_available;
    
public:

    virtual ~Square();

    /** Describes the different colors of squares. */
    enum ColorEnum
    {
        InvalidColor = 0,

        Light = 1,
        Dark = 2,

        /** For extending the board's functionality. */
        CustomColorOffset = 10
    };
    
    /** This is used to reference the squares surrounding this one. */
    enum RelativeSquareEnum
    {
        InvalidSquare = 0,
        
        North = 1,
        NorthEast = 2,
        East = 3,
        SouthEast = 4,
        South = 5,
        SouthWest = 6,
        West = 7,
        NorthWest = 8,
        
        CustomSquareOffset = 10
    };

    /** Describes the color of the square. */
    PROPERTY(Color, ColorEnum);
    
    /** References the piece that is on the square (if any). */
    PROPERTY_POINTER(Piece, Piece);
    
    /** Returns the square relative to this one. */
    Square const *GetSquare(RelativeSquareEnum) const;
    
    /** Returns the square relative to this one. */
    Square *GetSquare(RelativeSquareEnum);

    /** Returns the board to which this square belongs. */
    inline Board const *GetBoard() const{ return board; }
    
    /** Returns true if en passant is available on the square. */
    inline bool EnPassantAvailable() const{ return en_passant; }
    
    /** Returns true if the king may castle on this square. */
    inline bool CastleAvailable() const{ return castle_available; }

    /** Returns true if there is no piece on the square. */
    inline bool IsEmpty() const{ return NULL == GetPiece(); }

    /** Returns true if the squares are the same. */
    bool operator == (const Square &other);

    /** Returns true if the squares are not the same. */
    bool operator != (const Square &other);
    
    
private:

    Square(Board *parent_board, ColorEnum);

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_SQUARE_H
