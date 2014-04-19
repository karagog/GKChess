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

NAMESPACE_GKCHESS;


/** Maps col-row indices to a 1-dimensional index. */
static int __map_2d_indices_to_1d(GUINT32 col, GUINT32 row)
{
    return (col << 3) | row;
}



Board::Board(int num_cols)
    :m_columnCount(num_cols)
{
    _init();
}

Board::Board(const Board &o)
    :m_columnCount(o.ColumnCount())
{
    _copy_construct(o);
}

Board::Board(const AbstractBoard &o)
    :m_columnCount(o.ColumnCount())
{
    _copy_construct(o);
}

Board &Board::operator = (const AbstractBoard &o)
{
    _copy_construct(o);
    return *this;
}

void Board::_init()
{
    // Initialize all the squares in the array
    m_squares = (Square *)malloc(ColumnCount() * RowCount() * sizeof(Square));
    for(int c = 0; c < ColumnCount(); ++c)
        for(int r = 0; r < RowCount(); ++r)
            new(&square_at(c, r)) Square(c, r);
}

void Board::_copy_construct(const AbstractBoard &o)
{
    if(ColumnCount() != o.ColumnCount() || RowCount() != o.RowCount())
        THROW_NEW_GUTIL_EXCEPTION2(GUtil::Exception, "Board size mismatch");
    _init();
    _copy_board(o);
    _init_index();
}

void Board::_copy_board(const AbstractBoard &o)
{
    for(int c = 0; c < ColumnCount(); ++c)
    {
        for(int r = 0; r < RowCount(); ++r)
        {
            square_at(c, r) = o.SquareAt(c, r);
        }
    }
}

void Board::_init_index()
{
    m_index.Clear();
    for(int c = 0; c < ColumnCount(); ++c)
    {
        for(int r = 0; r < RowCount(); ++r)
        {
            const Square &cur = SquareAt(c, r);
            if(cur.GetPiece())
                m_index.InsertMulti(cur.GetPiece(), &cur);
        }
    }
}

Board::~Board()
{
    free(m_squares);
}

int Board::ColumnCount() const
{
    return m_columnCount;
}

int Board::RowCount() const
{
    return 8;
}

Square const &Board::SquareAt(int col, int row) const
{
    return m_squares[__map_2d_indices_to_1d(col, row)];
}

Square &Board::square_at(int c, int r)
{
    return m_squares[__map_2d_indices_to_1d(c, r)];
}

void Board::SetPiece(Piece const &p, const Square &s)
{
    Piece orig = s.GetPiece();
    AbstractBoard::SetPiece(p, s);

    // Remove the old piece from the index
    if(!orig.IsNull())
        m_index.Remove(orig, &s);
    // Add the new piece to the index
    if(!p.IsNull())
        m_index.InsertMulti(p, &s);
}

Vector<Square const *> Board::FindPieces(Piece const &pc) const
{
    Vector<Square const *> ret;
    if(Piece::NoPiece == pc.GetType()){
        if(Piece::AnyAllegience == pc.GetAllegience()){
            // Append all pieces to the list
            for(typename Map<Piece, SquarePointerConst>::const_iterator iter = m_index.begin();
                iter != m_index.end(); ++iter){
                G_FOREACH_CONST(Square const *sqr, iter->Values())
                    ret.PushBack(sqr);
            }
        }
        else{
            // Append only pieces of the given allegience to the list
            for(typename Map<Piece, Square const *>::const_iterator iter = m_index.begin();
                iter != m_index.end(); ++iter){
                if(iter->Key().GetAllegience() == pc.GetAllegience()){
                    G_FOREACH_CONST(Square const *sqr, iter->Values())
                        ret.PushBack(sqr);
                }
            }
        }
    }
    else{
        ret = m_index.Values(pc);
    }
    return ret;
}



ObservableBoard::ObservableBoard(int num_cols, QObject *p)
    :QObject(p),
      Board(num_cols)
{}

ObservableBoard::ObservableBoard(const AbstractBoard &o)
    :QObject(0),
      Board(o)
{}

ObservableBoard &ObservableBoard::operator = (const AbstractBoard &o)
{
    return static_cast<ObservableBoard &>(Board::operator = (o));
}

ObservableBoard::~ObservableBoard()
{}

void ObservableBoard::SetPiece(const Piece &p, const Square &s)
{
    emit NotifySquareAboutToBeUpdated(s);
    Board::SetPiece(p, s);
    emit NotifySquareUpdated(s);
}

void ObservableBoard::move_p(const MoveData &md)
{
    emit NotifyPieceAboutToBeMoved(md);
    Board::move_p(md);
    emit NotifyPieceMoved(md);
}


END_NAMESPACE_GKCHESS;
