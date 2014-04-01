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
      m_currentTurn(Piece::White),
      m_halfMoveClock(0),
      m_fullMoveNumber(0),
      m_enPassantSquare(0),
      m_whiteCastleInfo(0),
      m_blackCastleInfo(0)
{
    m_squares.ReserveExactly(64);
    for(int c = 0; c < ColumnCount(); ++c)
        for(int r = 0; r < RowCount(); ++r)
            m_squares.PushBack(Square(c, r));
}

Board::Board(const AbstractBoard &o)
    :AbstractBoard(o.parent()),
      m_currentTurn(o.GetWhoseTurn()),
      m_halfMoveClock(o.GetHalfMoveClock()),
      m_fullMoveNumber(o.GetFullMoveNumber()),
      m_enPassantSquare(o.GetEnPassantSquare()),
      m_whiteCastleInfo(o.GetCastleInfo(Piece::White)),
      m_blackCastleInfo(o.GetCastleInfo(Piece::Black))
{
    if(ColumnCount() != o.ColumnCount() || RowCount() != o.RowCount())
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Cannot copy from different sized board");
    for(int c = 0; c < ColumnCount(); ++c){
        for(int r = 0; r < RowCount(); ++r){
            Piece const *p = o.GetPiece(c, r);
            if(p)
                SetPiece(*p, c, r);
        }
    }
}

Board &Board::operator = (const AbstractBoard &o)
{
    this->~Board();
    new(this) Board(o);
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

void Board::MovePiece(int s_col, int s_row, int d_col, int d_row)
{
    Square &s(m_squares[__map_2d_indices_to_1d(s_col, s_row)]);
    Square &d(m_squares[__map_2d_indices_to_1d(d_col, d_row)]);
    Piece const *p = s.GetPiece();
    if(s != d && NULL != p)
    {
        d.SetPiece(*p);
        s.SetPiece(Piece());

        // Notify that the piece moved
        emit NotifyPieceMoved(*p, s_col, s_row, d_col, d_row);
    }
}

ISquare const &Board::SquareAt(int column, int row) const
{
    return m_squares[__map_2d_indices_to_1d(column, row)];
}


END_NAMESPACE_GKCHESS;
