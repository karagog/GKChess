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
      m_row(r)
{}

int Board::Square::GetColumn() const
{
    return m_column; 
}

int Board::Square::GetRow() const
{
    return m_row; 
}

Piece const *Board::Square::GetPiece() const
{
    return m_piece.GetType() == Piece::NoPiece ? 0 : &m_piece;
}

void Board::Square::SetPiece(const Piece &p)
{
    m_piece = p;
}



Board::Board(QObject *parent)
    :AbstractBoard(parent),
      m_squares(ColumnCount() * RowCount()),
      m_enPassantSquare(0),
      m_whiteCastleInfo(0),
      m_blackCastleInfo(0)
{
    for(int c = 0; c < ColumnCount(); ++c)
        for(int r = 0; r < RowCount(); ++r)
            m_squares.PushBack(Square(c, r));
}

Board::~Board()
{}

int Board::ColumnCount() const
{
    return 8;
}

int Board::RowCount() const
{
    return 8;
}

ISquare const *Board::GetEnPassantSquare() const
{
    return m_enPassantSquare;
}

void Board::SetEnPassantSquare(const ISquare *s)
{
    m_enPassantSquare = s;
}

GUINT8 Board::GetCastleInfo(Piece::AllegienceEnum a) const
{
    return Piece::White == a ? m_whiteCastleInfo : m_blackCastleInfo;
}

void Board::SetCastleInfo(Piece::AllegienceEnum a, GUINT8 info)
{
    switch(a)
    {
    case Piece::White:
        m_whiteCastleInfo = info;
        break;
    case Piece::Black:
        m_blackCastleInfo = info;
        break;
    }
}

/** Maps col-row indices to a 1-dimensional index. */
static int __map_2d_indices_to_1d(int col, int row)
{
    return (col << 3) & row;
}

void Board::SetPiece(const Piece &p, int column, int row)
{
    m_squares[__map_2d_indices_to_1d(column, row)].SetPiece(p);
    emit NotifySquareUpdated(column, row);
}

ISquare const &Board::SquareAt(int column, int row) const
{
    return m_squares[__map_2d_indices_to_1d(column, row)];
}


END_NAMESPACE_GKCHESS;
