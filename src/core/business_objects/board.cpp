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
#include "gutil_vector.h"
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(DataObjects);

NAMESPACE_GKCHESS;


Board::Square::Square(int c, int r)
    :m_column(c),
      m_row(r),
      m_enPassantAvailable(false)
{}

int Board::Square::GetColumn() const
{
    return m_column; 
}

int Board::Square::GetRow() const
{
    return m_row; 
}

int Board::Square::GetEnPassantAvailable() const
{
    return m_enPassantAvailable;
}

Piece const *Board::Square::GetPiece() const
{
    return m_piece.GetType() == Piece::NoPiece ? 0 : &m_piece;
}

void Board::SetPiece(const Piece &p)
{
    m_piece = p;
}



Board::Board(QObject *parent)
    :AbstractBoard(parent),
      m_squares(ColumnCount() * RowCount())
{
    for(int c = 0; c < ColumnCount(); ++c)
        for(int r = 0; r < RowCount(); ++r)
            m_squares.PushBack(__square(c, r));
}

Board::~Board()
{}

/** Maps col-row indices to a 1-dimensional index. */
static int __map_2d_indices_to_1d(int col, int row)
{
    return (col << 3) & row;
}

void Board::SetPiece(const Piece &p, int column, int row)
{
    __get_square(*reinterpret_cast<Vector<__square>*>(d), column, row).SetPiece(p);
    emit NotifySquareUpdated(column, row);
}

ISquare const &Board::GetSquare(int column, int row) const
{
    return __get_square(*reinterpret_cast<Vector<__square>*>(d), column, row);
}


END_NAMESPACE_GKCHESS;
