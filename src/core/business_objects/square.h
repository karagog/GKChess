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

#include "gkchess_piece.h"

NAMESPACE_GKCHESS;


/** Describes one square of the chess board.
 *  This is an interface because its implementation depends on
 *  the board's implementation
*/
class Square
{
    template<GUINT32 NUM_COLS> friend class Board_Imp;

    // For fast lookups this is implemented as quickly queryable data types
    const GUINT8 m_column, m_row;
    GINT8 m_threatsWhite, m_threatsBlack;
    Piece m_piece;

    Square(GUINT8 col, GUINT8 row);

    /** Copies the square's data, except the row and column. */
    Square &operator = (const Square &o);

public:

    /** Holds the column that the square is in. */
    int GetColumn() const;

    /** Holds the row that the square is in. */
    int GetRow() const;

    /** Returns the piece on the square.  The piece will be null if this is an empty square. */
    Piece const &GetPiece() const;

    /** Sets the piece on the square. */
    void SetPiece(Piece const &);

    /** Returns the number of threats on this square by the given allegience.
        It may return -1, indicating that the threat count hasn't been computed.
    */
    int GetThreatCount(Piece::AllegienceEnum) const;

    /** Sets the threat count for the square for the given allegience. */
    void SetThreatCount(Piece::AllegienceEnum, int);

    /** Compares squares based on their row and column.
     *  You should implement an equality comparer in the most efficient way for your
     *  board implementation.
    */
    bool operator == (const Square &other) const;

    /** Compares squares based on their row and column.
     *  You should implement an equality comparer in the most efficient way for your
     *  board implementation.
    */
    bool operator != (const Square &other) const;


    /** A convenience function that tells you if this square is light or dark. */
    bool IsDarkSquare() const;

    /** Returns string form of the square.  For example, "e4". */
    GUtil::String ToString() const;

};


typedef Square const *SquarePointerConst;
typedef Square *SquarePointer;


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_SQUARE_H
