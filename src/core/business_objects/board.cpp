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

#include "board.h"
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(DataObjects);

NAMESPACE_GKCHESS;


Board::Board(GINT32 columns, GINT32 rows)
    :m_board(columns)
{
    _init(columns, rows);
}

Board::Board(const Board &b)
    :m_board(b.ColumnCount())
{
    _init(b.ColumnCount(), b.RowCount());

    // Copy the pieces from the other board:
    for(GUINT32 i = 0; i < b.ColumnCount(); ++i)
    {
        for(GUINT32 j = 0; j < b.RowCount(); ++j)
        {
            Square const &other_square(b.m_board[i][j]);
            Square &this_square(m_board[i][j]);

            if(other_square.GetPiece())
                this_square.SetPiece(new Piece(other_square.GetPiece()->GetAllegience(), other_square.GetPiece()->GetType()));
        }
    }
}

Board &Board::operator = (const Board &other)
{
    this->~Board();
    new(this) Board(other);
    return *this;
}

void Board::_init(GINT32 columns, GINT32 rows)
{
    if(0 > rows || 0 > columns)
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Rows and Columns must be positive");

    // First create the board
    for(GINT32 i = 0; i < columns; ++i)
    {
        m_board.PushBack(Vector<Square>(rows));
        for(GINT32 j = 0; j < rows; ++j){
            m_board[i].PushBack(Square(this, ((OddOrEven(i) && OddOrEven(j)) || (!OddOrEven(i) && !OddOrEven(j))) ?
                                                Square::Dark : Square::Light));
        }
    }

    // Then wire the squares together so they have spacial awareness
    for(GINT32 i = 0; i < columns; ++i)
    {
        for(GINT32 j = 0; j < rows; ++j)
        {
            Square &s( m_board[i][j] );

            // Each square knows its row and column
            s._p_Column = i;
            s._p_Row = j;

            if(rows > j + 1)
                s.north = &m_board[i][j + 1];
            if(rows > j + 1 && columns > i + 1)
                s.north_east = &m_board[i + 1][j + 1];
            if(columns > i + 1)
                s.east = &m_board[i + 1][j];
            if(0 <= j - 1 && columns > i + 1)
                s.south_east = &m_board[i + 1][j - 1];
            if(0 <= j - 1)
                s.south = &m_board[i][j - 1];
            if(0 <= j - 1 && 0 <= i - 1)
                s.south_west = &m_board[i - 1][j - 1];
            if(0 <= i - 1)
                s.west = &m_board[i - 1][j];
            if(rows > j + 1 && 0 <= i - 1)
                s.north_west = &m_board[i - 1][j + 1];
        }
    }
}

Board::~Board()
{
    // Need to delete all the pieces we set up
    Clear();
}

void Board::Clear()
{
    for(GUINT32 i = 0; i < ColumnCount(); ++i){
        for(GUINT32 j = 0; j < RowCount(); ++j)
        {
            Square &cur( m_board[i][j] );
            if(cur.GetPiece())
            {
                delete cur.GetPiece();
                cur.SetPiece(0);
            }
        }
    }
}

bool Board::IsEmpty() const
{
    bool ret = true;
    for(GUINT32 i = 0; i < ColumnCount() && ret; ++i){
        for(GUINT32 j = 0; j < RowCount() && ret; ++j){
            if(m_board[i][j].GetPiece())
                ret = false;
        }
    }
    return ret;
}

GUINT32 Board::RowCount() const
{
    return 0 < m_board.Length() ? m_board[0].Length() : 0;
}

GUINT32 Board::ColumnCount() const
{
    return m_board.Length();
}

Square const &Board::GetSquare(GUINT32 column, GUINT32 row) const
{
    return m_board[column][row];
}


END_NAMESPACE_GKCHESS;
