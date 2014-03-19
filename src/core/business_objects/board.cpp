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
      m_currentTurn(Piece::White),
      m_halfMoveClock(0),
      m_fullMoveNumber(0),
      m_enPassantSquare(0),
      m_whiteCastleInfo(0),
      m_blackCastleInfo(0)
{
    for(int c = 0; c < ColumnCount(); ++c)
        for(int r = 0; r < RowCount(); ++r)
            m_squares.PushBack(Square(c, r));
}

Board::Board(const Board &o)
    :AbstractBoard(o.parent()),
      m_squares(o.m_squares),
      m_currentTurn(o.m_currentTurn),
      m_halfMoveClock(o.m_halfMoveClock),
      m_fullMoveNumber(o.m_fullMoveNumber),
      m_enPassantSquare(o.m_enPassantSquare),
      m_whiteCastleInfo(o.m_whiteCastleInfo),
      m_blackCastleInfo(o.m_blackCastleInfo)
{}

Board &Board::operator = (const Board &o)
{
    m_squares = o.m_squares;
    m_currentTurn = o.m_currentTurn;
    m_halfMoveClock = o.m_halfMoveClock;
    m_fullMoveNumber = o.m_fullMoveNumber;
    m_enPassantSquare = o.m_enPassantSquare;
    m_whiteCastleInfo = o.m_whiteCastleInfo;
    m_blackCastleInfo = o.m_blackCastleInfo;
    return *this;
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

Piece::AllegienceEnum Board::GetWhoseTurn() const
{
    return m_currentTurn;
}

void Board::SetWhoseTurn(Piece::AllegienceEnum a)
{
    m_currentTurn = a;
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
    default:
        break;
    }
}

int Board::GetHalfMoveClock() const
{
    return m_halfMoveClock;
}

void Board::SetHalfMoveClock(int h)
{
    m_halfMoveClock = h;
}

int Board::GetFullMoveNumber() const
{
    return m_fullMoveNumber;
}

void Board::SetFullMoveNumber(int f)
{
    m_fullMoveNumber = f;
}

/** Maps col-row indices to a 1-dimensional index. */
static int __map_2d_indices_to_1d(int col, int row)
{
    return (col << 3) | row;
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
