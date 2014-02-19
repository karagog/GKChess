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
#include "gkchess_square.h"
#include "gutil_vector.h"

NAMESPACE_GKCHESS;


/** Describes a chess board. */
class Board
{
public:

    Board(GINT32 columns = 8, GINT32 rows = 8);
    Board(const Board &);
    virtual ~Board();

    /** Returns the number of rows. */
    GUINT32 RowCount() const;

    /** Returns the number of columns. */
    GUINT32 ColumnCount() const;

    /** Removes all pieces from the board.
     *  \note This does not resize the board - The only way to do that would be to create a new board.
    */
    void Clear();

    /** Returns true if there are no pieces on the board. */
    bool IsEmpty() const;


    /** Returns a reference to the square at the given column and row.
     *  It does not check your bounds for performance reasons.
    */
    Square const &GetSquare(GUINT32 column, GUINT32 row) const;


    /** Returns a column of squares in an array. */
    inline Square *operator [] (int column){ return m_board[column]; }

    /** Returns a column of squares in an array. */
    inline Square const *operator [] (int column) const{ return m_board[column]; }

    Board &operator = (const Board &);


private:

    ::GUtil::DataObjects::Vector< ::GUtil::DataObjects::Vector<Square> > m_board;

    void _init(GINT32 columns, GINT32 rows);

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_BOARD_H
