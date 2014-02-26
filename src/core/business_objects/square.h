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
#include "gutil_smartpointer.h"

NAMESPACE_GKCHESS;

class Piece;


/** Describes one square of the chess board. */
class Square
{
    GUtil::Utils::SharedSmartPointer<Piece> piece;
public:

    /** Holds the column that the square is in. */
    READONLY_PROPERTY(Column, int);

    /** Holds the row that the square is in. */
    READONLY_PROPERTY(Row, int);

    /** Returns true if en passant is available on the square. */
    PROPERTY(EnPassantAvailable, bool);

    /** Returns true if the king may castle on this square. */
    PROPERTY(CastleAvailable, bool);


    /** Constructing a null square is meaningless, so this only exists to support
     *  arrays of squares. But you must later initialize it with the assignment operator.
    */
    Square();

    /** The main constructor for a square. You must tell it what row and column it is. */
    Square(int column, int row);

    Square(const Square &);
    Square &operator = (const Square &);

    virtual ~Square();

    
    /** References the piece that is on the square (if any). */
    Piece const *GetPiece() const{ return piece; }
    /** References the piece that is on the square (if any). */
    Piece *GetPiece(){ return piece; }

    /** Sets the piece on the square.
     *  The pieces are shared explicitly between boards, to support not having to
     *  reallocate a set of pieces for every board simulation.
    */
    void SetPiece(Piece *);

    /** Returns true if there is no piece on the square. */
    bool IsEmpty() const{ return NULL == GetPiece(); }

    /** A convenience function that tells you if this square is light or dark. */
    bool IsDarkSquare() const{ return (0x1 & GetRow()) == (0x1 & GetColumn()); }

    /** Returns true if the squares are the same. */
    bool operator == (const Square &other);

    /** Returns true if the squares are not the same. */
    bool operator != (const Square &other);

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_SQUARE_H
