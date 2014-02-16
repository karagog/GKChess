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

#include "gkchess_piece.h"
#include "gutil_vector.h"

NAMESPACE_GKCHESS;


/** Describes a chess board. */
class Board
{
public:

    Board(GINT32 rows = 8, GINT32 columns = 8);
    virtual ~Board();

    /** Returns the number of rows. */
    GUINT32 GetRows() const;

    /** Returns the number of columns. */
    GUINT32 GetColumns() const;

    /** Describes different ways the board could be set up. */
    enum SetupTypeEnum
    {
        /** Causes the board to be cleared of all pieces. */
        Empty = 0,

        /** A standard game of chess. */
        StandardChess = 1,

        /** You can create your own custom board setups starting from this offset. */
        CustomSetupOffset = 100
    };

    /** Sets up the board with the given type. */
    virtual void Setup(SetupTypeEnum = StandardChess);

    /** Removes all pieces from the board.
     *  \note This does not resize the board - The only way to do that would be to create a new board.
    */
    void Clear();

    /** Returns true if there are no pieces on the board. */
    bool IsEmpty() const;


    /** Describes one square of the chess board. */
    class Square
    {
        friend class Board;

        Square *north, *north_east, *east, *south_east, *south, *south_west, *west, *north_west;

    public:

        /** Describes the different colors of squares. */
        enum ColorEnum
        {
            InvalidColor = 0,

            Light = 1,
            Dark = 2,

            /** For extending the board's functionality. */
            CustomColorOffset = 10
        };

        explicit Square(ColorEnum);

        PROPERTY(Color, ColorEnum);
        PROPERTY_POINTER(Piece, Piece);

        /** Returns true if there is no piece on the square. */
        inline bool IsEmpty() const{ return NULL == GetPiece(); }

    };


    /** Returns a reference to the square at the given column and row.
     *  It does not check your bounds for performance reasons.
    */
    Square const &GetSquare(GUINT32 column, GUINT32 row) const;


private:

    ::GUtil::DataObjects::Vector< ::GUtil::DataObjects::Vector<Square> > m_board;

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_BOARD_H
