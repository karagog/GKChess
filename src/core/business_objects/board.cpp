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


/** Maps col-row indices to a 1-dimensional index. */
static int __map_2d_indices_to_1d(int col, int row)
{
    return (col << 3) | row;
}



Board::GameState::GameState()
    :WhoseTurn(Piece::AnyAllegience),
      CastleWhite1(-1),
      CastleWhite2(-1),
      CastleBlack1(-1),
      CastleBlack2(-1),
      EnPassantSquare(0),
      HalfMoveClock(-1),
      FullMoveNumber(-1)
{}

Board::GameState::GameState(const IGameState &o)
    :WhoseTurn(o.GetWhoseTurn()),
      CastleWhite1(o.GetCastleWhite1()),
      CastleWhite2(o.GetCastleWhite2()),
      CastleBlack1(o.GetCastleBlack1()),
      CastleBlack2(o.GetCastleBlack2()),
      EnPassantSquare(o.GetEnPassantSquare()),
      HalfMoveClock(o.GetHalfMoveClock()),
      FullMoveNumber(o.GetFullMoveNumber())
{}


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
    :AbstractBoard(parent)
{
    _init();
}

Board::Board(const AbstractBoard &o)
    :AbstractBoard(o.parent()),
      m_gameState(o.GameState())
{
    if(ColumnCount() != o.ColumnCount() || RowCount() != o.RowCount())
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Cannot copy from different sized board");

    _init();

    for(int c = 0; c < ColumnCount(); ++c){
        for(int r = 0; r < RowCount(); ++r){
            Piece const *p = o.GetPiece(c, r);
            if(p)
                set_piece_p(*p, c, r);
        }
    }
}

void Board::_init()
{
    m_squares.ReserveExactly(64);
    for(int c = 0; c < ColumnCount(); ++c)
        for(int r = 0; r < RowCount(); ++r)
            m_squares.PushBack(Square(c, r));
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

void Board::set_piece_p(const Piece &p, int col, int row)
{
    Square &sqr(_square_at(col, row));
    Piece const *piece_orig = sqr.GetPiece();

    // Remove the old piece from the index
    if(piece_orig)
        _index(piece_orig->GetAllegience()).Remove(piece_orig->GetType(), &sqr);

    // Add the new piece to the index
    if(!p.IsNull())
        _index(p.GetAllegience()).InsertMulti(p.GetType(), &sqr);

    sqr.SetPiece(p);
}

void Board::move_p(const MoveData &md)
{
    Square &s(_square_at(md.Source->GetColumn(), md.Source->GetRow()));
    Square &d(_square_at(md.Destination->GetColumn(), md.Destination->GetRow()));

    Piece const *piece_orig = s.GetPiece();
    Piece const *piece_dest = d.GetPiece();
    Map<Piece::PieceTypeEnum, ISquare const *> &index(_index(piece_orig->GetAllegience()));

    // Remove the piece at the dest square from the index
    if(piece_dest){
        index.Remove(piece_dest->GetType(), &d);
    }

    // Update the index for the piece being moved
    if(piece_orig)
    {
        // If the piece being moved is not the same piece as the source square (corner case)
        if(*piece_orig != md.PieceMoved)
        {
            // This section of code should not execute unless there is a bug in our move logic
            GASSERT(false);
            index.Remove(piece_orig->GetType(), &s);
            _index(md.PieceMoved.GetAllegience()).InsertMulti(md.PieceMoved.GetType(), &d);
        }
        else
            index[piece_orig->GetType()] = &d;
    }

    s.SetPiece(Piece());
    d.SetPiece(md.PieceMoved);
}

ISquare const &Board::SquareAt(int col, int row) const
{
    return m_squares[__map_2d_indices_to_1d(col, row)];
}

Board::Square &Board::_square_at(int col, int row)
{
    return m_squares[__map_2d_indices_to_1d(col, row)];
}

Map<Piece::PieceTypeEnum, ISquare const *> &Board::_index(Piece::AllegienceEnum a)
{
    return Piece::White == a ? m_whitePieceIndex : m_blackPieceIndex;
}

Map<Piece::PieceTypeEnum, ISquare const *> const &Board::_index(Piece::AllegienceEnum a) const
{
    return Piece::White == a ? m_whitePieceIndex : m_blackPieceIndex;
}

Vector<ISquare const *> Board::FindPieces(const Piece &pc) const
{
    Vector<ISquare const *> ret( (Piece::White == pc.GetAllegience() ?
                                      &m_whitePieceIndex : &m_blackPieceIndex)->Values(pc.GetType()) );

    // To help debug, make sure all the returned pieces are the correct type
    for(GINT32 i = 0; i < ret.Length(); ++i){
        Piece const *p = ret[i]->GetPiece();
        GUTIL_UNUSED(p);
        GASSERT(p && pc.GetType() == p->GetType() && pc.GetAllegience() == p->GetAllegience());
    }

    return ret;
}


END_NAMESPACE_GKCHESS;
