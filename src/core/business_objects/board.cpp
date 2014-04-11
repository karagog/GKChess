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
#include "gkchess_piece.h"
#include "gutil_map.h"
USING_NAMESPACE_GUTIL;

#define NUM_COLS 8
#define NUM_ROWS 8

NAMESPACE_GKCHESS;


/** Maps col-row indices to a 1-dimensional index. */
static int __map_2d_indices_to_1d(int col, int row)
{
    return (col << 3) | row;
}


/** Our square implementation. */
class Square : public ISquare
{
    // For fast lookups this is implemented as quickly queryable data types
    Piece m_piece;
    int m_column;
    int m_row;
public:

    Square(int c, int r):
        m_column(c),
        m_row(r)
    {}
    int GetColumn() const{ return m_column; }
    int GetRow() const{ return m_row; }
    Piece const *GetPiece() const{ return m_piece.GetType() == Piece::NoPiece ? 0 : &m_piece; }

    /** This function is not part of the interface, but it's necessary anyways */
    void SetPiece(const Piece &p){ m_piece = p; }
};


/** This function maps a 1-D array into a 2-D array. */
static Square &__square_at(Square *sa, int col, int row)
{
    return sa[__map_2d_indices_to_1d(col, row)];
}

/** This function maps a 1-D array into a 2-D array. */
static Square const &__square_at(Square const *sa, int col, int row)
{
    return sa[__map_2d_indices_to_1d(col, row)];
}


/** Our game state implementation. */
class GameState : public IGameState
{
    Piece::AllegienceEnum WhoseTurn;
    int CastleWhite1;
    int CastleWhite2;
    int CastleBlack1;
    int CastleBlack2;
    ISquare const *EnPassantSquare;
    int HalfMoveClock;
    int FullMoveNumber;
public:
    Piece::AllegienceEnum GetWhoseTurn() const{ return WhoseTurn; }
    void SetWhoseTurn(Piece::AllegienceEnum v){ WhoseTurn=v; }
    int GetCastleWhite1() const{ return CastleWhite1; }
    void SetCastleWhite1(int v){ CastleWhite1=v; }
    int GetCastleWhite2() const{ return CastleWhite2; }
    void SetCastleWhite2(int v){ CastleWhite2=v; }
    int GetCastleBlack1() const{ return CastleBlack1; }
    void SetCastleBlack1(int v){ CastleBlack1=v; }
    int GetCastleBlack2() const{ return CastleBlack2; }
    void SetCastleBlack2(int v){ CastleBlack2=v; }
    ISquare const *GetEnPassantSquare() const{ return EnPassantSquare; }
    void SetEnPassantSquare(ISquare const *v){ EnPassantSquare=v; }
    int GetHalfMoveClock() const{ return HalfMoveClock; }
    void SetHalfMoveClock(int v){ HalfMoveClock=v; }
    int GetFullMoveNumber() const{ return FullMoveNumber; }
    void SetFullMoveNumber(int v){ FullMoveNumber=v; }

    GameState():
        WhoseTurn(Piece::AnyAllegience),
        CastleWhite1(-1),
        CastleWhite2(-1),
        CastleBlack1(-1),
        CastleBlack2(-1),
        EnPassantSquare(0),
        HalfMoveClock(-1),
        FullMoveNumber(-1)
    {}
    GameState(const IGameState &o):
        WhoseTurn(o.GetWhoseTurn()),
        CastleWhite1(o.GetCastleWhite1()),
        CastleWhite2(o.GetCastleWhite2()),
        CastleBlack1(o.GetCastleBlack1()),
        CastleBlack2(o.GetCastleBlack2()),
        EnPassantSquare(o.GetEnPassantSquare()),
        HalfMoveClock(o.GetHalfMoveClock()),
        FullMoveNumber(o.GetFullMoveNumber())
  {}
};


struct g_d_t
{
    Vector<Square> squares;
    GameState gamestate;

    // These facilitate fast piece lookups
    Map<Piece::PieceTypeEnum, ISquare const *> index_white;
    Map<Piece::PieceTypeEnum, ISquare const *> index_black;
};


static Map<Piece::PieceTypeEnum, ISquare const *> &__index(g_d_t *d, Piece::AllegienceEnum a)
{
    return Piece::White == a ? d->index_white : d->index_black;
}


Board::Board(QObject *parent)
    :AbstractBoard(parent)
{
    _init();
}

Board::Board(const AbstractBoard &o)
    :AbstractBoard(o.parent())
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
    G_D_INIT();
    G_D;

    // Instantiate all the squares
    d->squares.ReserveExactly(NUM_COLS * NUM_ROWS);
    for(int c = 0; c < ColumnCount(); ++c)
        for(int r = 0; r < RowCount(); ++r)
            d->squares.PushBack(Square(c, r));
}

Board &Board::operator = (const AbstractBoard &o)
{
    this->~Board();
    new(this) Board(o);
    return *this;
}

Board::~Board()
{
    G_D_UNINIT();
}

int Board::ColumnCount() const
{
    return NUM_COLS;
}

int Board::RowCount() const
{
    return NUM_ROWS;
}

void Board::set_piece_p(const Piece &p, int col, int row)
{
    G_D;
    Square &sqr(__square_at(d->squares, col, row));
    Piece const *piece_orig = sqr.GetPiece();

    // Remove the old piece from the index
    if(piece_orig)
        __index(d, piece_orig->GetAllegience()).Remove(piece_orig->GetType(), &sqr);

    // Add the new piece to the index
    if(!p.IsNull())
        __index(d, p.GetAllegience()).InsertMulti(p.GetType(), &sqr);

    sqr.SetPiece(p);
}

void Board::move_p(const MoveData &md)
{
    G_D;
    Square &src(__square_at(d->squares, md.Source->GetColumn(), md.Source->GetRow()));
    Square &dest(__square_at(d->squares, md.Destination->GetColumn(), md.Destination->GetRow()));

    Piece const *piece_orig = src.GetPiece();
    Piece const *piece_dest = dest.GetPiece();
    Map<Piece::PieceTypeEnum, ISquare const *> &index(__index(d, piece_orig->GetAllegience()));

    // Remove the piece at the dest square from the index
    if(piece_dest){
        index.Remove(piece_dest->GetType(), &dest);
    }

    // Update the index for the piece being moved
    if(piece_orig)
    {
        // If the piece being moved is not the same piece as the source square (corner case)
        if(*piece_orig != md.PieceMoved)
        {
            // This section of code should not execute unless there is a bug in our move logic
            GASSERT(false);
            index.Remove(piece_orig->GetType(), &src);
            __index(d, md.PieceMoved.GetAllegience()).InsertMulti(md.PieceMoved.GetType(), &dest);
        }
        else
            index[piece_orig->GetType()] = &dest;
    }

    src.SetPiece(Piece());
    dest.SetPiece(md.PieceMoved);
}

ISquare const &Board::SquareAt(int col, int row) const
{
    G_D;
    return __square_at(d->squares, col, row);
}

Vector<ISquare const *> Board::FindPieces(const Piece &pc) const
{
    G_D;
    Vector<ISquare const *> ret( __index(d, pc.GetAllegience()).Values(pc.GetType()) );

    // To help debug, make sure all the returned pieces are the correct type
    for(GINT32 i = 0; i < ret.Length(); ++i){
        Piece const *p = ret[i]->GetPiece();
        GUTIL_UNUSED(p);
        GASSERT(p && pc.GetType() == p->GetType() && pc.GetAllegience() == p->GetAllegience());
    }

    return ret;
}

const IGameState &Board::GameState() const
{
    G_D;
    return d->gamestate;
}

IGameState &Board::GameState()
{
    G_D;
    return d->gamestate;
}


END_NAMESPACE_GKCHESS;
