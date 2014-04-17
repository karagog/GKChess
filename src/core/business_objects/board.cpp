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
    int m_threatsWhite, m_threatsBlack;
public:

    Square(int c, int r):
        m_column(c),
        m_row(r),
        m_threatsWhite(-1),
        m_threatsBlack(-1)
    {}
    
    int GetColumn() const{ return m_column; }
    int GetRow() const{ return m_row; }
    Piece const *GetPiece() const{ return m_piece.GetType() == Piece::NoPiece ? 0 : &m_piece; }
    void SetPiece(const Piece &p){ m_piece = p; }
    int GetThreatCount(Piece::AllegienceEnum a) const{ 
        return Piece::White == a ? m_threatsWhite : m_threatsBlack; 
    }
    void SetThreatCount(Piece::AllegienceEnum a, int c){
        if(Piece::White == a)
            m_threatsWhite = c;
        else
            m_threatsBlack = c;
    }
};


/** This function maps a 1-D array into a 2-D array. */
static Square &__square_at(Square *sa, int col, int row)
{
    return sa[__map_2d_indices_to_1d(col, row)];
}


/** Our game state data. */
struct GameState
{
    Piece::AllegienceEnum WhoseTurn;
    int CastleWhiteA;
    int CastleWhiteH;
    int CastleBlackA;
    int CastleBlackH;
    ISquare const *EnPassantSquare;
    int HalfMoveClock;
    int FullMoveNumber;
    Board::ResultTypeEnum Result;

    GameState():
        WhoseTurn(Piece::AnyAllegience),
        CastleWhiteA(-1),
        CastleWhiteH(-1),
        CastleBlackA(-1),
        CastleBlackH(-1),
        EnPassantSquare(0),
        HalfMoveClock(-1),
        FullMoveNumber(-1),
        Result(Board::Undecided)
    {}

    /** Constructs the game state object from another abstract board. */
    GameState(const AbstractBoard &o):
        WhoseTurn(o.GetWhoseTurn()),
        CastleWhiteA(o.GetCastleWhiteA()),
        CastleWhiteH(o.GetCastleWhiteH()),
        CastleBlackA(o.GetCastleBlackA()),
        CastleBlackH(o.GetCastleBlackH()),
        EnPassantSquare(o.GetEnPassantSquare()),
        HalfMoveClock(o.GetHalfMoveClock()),
        FullMoveNumber(o.GetFullMoveNumber()),
        Result(o.GetResult())
  {}

};


struct d_t
{
    Vector<Square> squares;
    GameState gamestate;

    // These facilitate fast piece lookups
    Map<Piece::PieceTypeEnum, ISquare const *> index_white;
    Map<Piece::PieceTypeEnum, ISquare const *> index_black;
};


static Map<Piece::PieceTypeEnum, ISquare const *> &__index(d_t *d, Piece::AllegienceEnum a)
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

    // copy the contents of the other board's squares
    for(int c = 0; c < ColumnCount(); ++c){
        for(int r = 0; r < RowCount(); ++r){
            square_at(c, r) = o.SquareAt(c, r);
        }
    }
}

void Board::_init()
{
    G_D_INIT();
    G_D;

    // Instantiate all the squares and initialize their row/column data
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
    ISquare &sqr(square_at(col, row));
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
    ISquare &src(square_at(md.Source->GetColumn(), md.Source->GetRow()));
    ISquare &dest(square_at(md.Destination->GetColumn(), md.Destination->GetRow()));

    Piece const *piece_orig = src.GetPiece();
    Piece const *piece_dest = dest.GetPiece();
    Map<Piece::PieceTypeEnum, ISquare const *> &index(__index(d, piece_orig->GetAllegience()));

    if(MoveData::NoCastle == md.CastleType)
    {
        // Remove the piece at the dest square from the index
        if(piece_dest){
            index.Remove(piece_dest->GetType(), &dest);
        }

        // Update the index for the piece being moved
        if(piece_orig)
        {
            // If the piece being moved is not the same piece as the source square (corner case)
            if(*piece_orig == md.PieceMoved)
                index.At(piece_orig->GetType(), &src) = &dest;
            else{
                GASSERT(false);
            }
        }

        src.SetPiece(Piece());
        dest.SetPiece(md.PieceMoved);
    }
    else
    {
        int rook_col_src, rook_col_dest;
        int rank;
        if(Piece::White == piece_orig->GetAllegience())
        {
            rank = 0;
            if(MoveData::CastleASide == md.CastleType)
            {
                rook_col_src = GetCastleWhiteA();
                rook_col_dest = 3;
            }
            else
            {
                rook_col_src = GetCastleWhiteH();
                rook_col_dest = 5;
            }
        }
        else
        {
            rank = 7;
            if(MoveData::CastleASide == md.CastleType)
            {
                rook_col_src = GetCastleBlackA();
                rook_col_dest = 3;
            }
            else
            {
                rook_col_src = GetCastleBlackH();
                rook_col_dest = 5;
            }
        }

        // Move the rook
        square_at(rook_col_src, rank).SetPiece(Piece());
        square_at(rook_col_dest, rank).SetPiece(Piece(Piece::Rook, piece_orig->GetAllegience()));

        // Move the King
        dest.SetPiece(*piece_orig);
        src.SetPiece(Piece());
    }
}

ISquare const &Board::SquareAt(int col, int row) const
{
    G_D;
    return __square_at(d->squares, col, row);
}

ISquare &Board::square_at(int col, int row)
{
    G_D;
    return __square_at(d->squares, col, row);
}

Vector<ISquare const *> Board::FindPieces(const Piece &pc) const
{
    G_D;
    Vector<ISquare const *> ret;
    if(Piece::NoPiece == pc.GetType())
    {
        if(Piece::AnyAllegience == pc.GetAllegience())
        {
            // Append all pieces to the list
            for(typename Map<Piece::PieceTypeEnum, ISquare const *>::const_iterator iter = d->index_white.begin();
                iter != d->index_white.end(); ++iter)
            {
                G_FOREACH_CONST(ISquare const *sqr, iter->Values())
                    ret.PushBack(sqr);
            }
            
            for(typename Map<Piece::PieceTypeEnum, ISquare const *>::const_iterator iter = d->index_black.begin();
                iter != d->index_black.end(); ++iter)
            {
                G_FOREACH_CONST(ISquare const *sqr, iter->Values())
                    ret.PushBack(sqr);
            }
        }
        else
        {
            // Append only pieces of the given allegience to the list
            for(typename Map<Piece::PieceTypeEnum, ISquare const *>::const_iterator iter = __index(d, pc.GetAllegience()).begin();
                iter != __index(d, pc.GetAllegience()).end(); ++iter)
            {
                G_FOREACH_CONST(ISquare const *sqr, iter->Values())
                    ret.PushBack(sqr);
            }
        }
    }
    else
    {
        ret = __index(d, pc.GetAllegience()).Values(pc.GetType());
    }

    // To help debug, make sure all the returned pieces are the correct type
    for(GINT32 i = 0; i < ret.Length(); ++i){
        Piece const *p = ret[i]->GetPiece();
        GUTIL_UNUSED(p);
        GASSERT(p && (Piece::NoPiece == pc.GetType() || pc.GetType() == p->GetType()) && pc.GetAllegience() == p->GetAllegience());
    }

    return ret;
}

Piece::AllegienceEnum Board::GetWhoseTurn() const
{
    G_D;
    return d->gamestate.WhoseTurn;
}
void Board::SetWhoseTurn(Piece::AllegienceEnum v)
{
    G_D;
    d->gamestate.WhoseTurn=v;
}
int Board::GetCastleWhiteA() const
{
    G_D;
    return d->gamestate.CastleWhiteA;
}
void Board::SetCastleWhiteA(int v)
{
    G_D;
    d->gamestate.CastleWhiteA=v;
}
int Board::GetCastleWhiteH() const
{
    G_D;
    return d->gamestate.CastleWhiteH;
}
void Board::SetCastleWhiteH(int v)
{
    G_D;
    d->gamestate.CastleWhiteH=v;
}
int Board::GetCastleBlackA() const
{
    G_D;
    return d->gamestate.CastleBlackA;
}
void Board::SetCastleBlackA(int v)
{
    G_D;
    d->gamestate.CastleBlackA=v;
}
int Board::GetCastleBlackH() const
{
    G_D;
    return d->gamestate.CastleBlackH;
}
void Board::SetCastleBlackH(int v)
{
    G_D;
    d->gamestate.CastleBlackH=v;
}
ISquare const *Board::GetEnPassantSquare() const
{
    G_D;
    return d->gamestate.EnPassantSquare;
}
void Board::SetEnPassantSquare(ISquare const *v)
{
    G_D;
    d->gamestate.EnPassantSquare=v;
}
int Board::GetHalfMoveClock() const
{
    G_D;
    return d->gamestate.HalfMoveClock;
}
void Board::SetHalfMoveClock(int v)
{
    G_D;
    d->gamestate.HalfMoveClock=v;
}
int Board::GetFullMoveNumber() const
{
    G_D;
    return d->gamestate.FullMoveNumber;
}
void Board::SetFullMoveNumber(int v)
{
    G_D;
    d->gamestate.FullMoveNumber=v;
}
Board::ResultTypeEnum Board::GetResult() const
{
    G_D;
    return d->gamestate.Result;
}
void Board::SetResult(ResultTypeEnum r)
{
    G_D;
    d->gamestate.Result = r;
}


END_NAMESPACE_GKCHESS;
