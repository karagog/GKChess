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
#include "gutil_flags.h"
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(DataObjects);

NAMESPACE_GKCHESS;


/** Our square implementation. */
class __square :
        public Square
{
    enum SquareInfoEnum
    {
        PieceType_Start = 0,
        PieceType_End = 2,
        PieceAllegience = 3,
        EnPassantAvailable = 4,

        Column_Start = 26,
        Column_End = 28,
        Row_Start = 29,
        Row_End = 31
    };

public:

    GUTIL_DECLARE_FLAGS2(SquareInfoFlags, SquareInfoEnum, GUINT32);

    __square(int col, int row)
        :m_info((GUINT32)Piece::NoPiece)
    {
        m_info.SetRange(Column_Start, Column_End, col);
        m_info.SetRange(Row_Start, Row_End, row);
    }

    virtual int GetColumn() const{ return m_info.TestRange(Column_Start, Column_End); }
    virtual int GetRow() const{ return m_info.TestRange(Row_Start, Row_End); }
    virtual bool GetEnPassantAvailable() const{ return m_info.TestFlag(EnPassantAvailable); }

    virtual Piece GetPiece() const{
        return Piece((Piece::PieceTypeEnum)m_info.TestRange(PieceType_Start, PieceType_End),
                     m_info.TestFlag(PieceAllegience) ? Piece::Black : Piece::White);
    }


    void SetPiece(const Piece &p){
        m_info.SetRange(PieceType_Start, PieceType_End, p.GetType());
        m_info.SetFlag(PieceAllegience, p.GetAllegience());
    }


private:
    SquareInfoFlags m_info;
};



Board::Board(QObject *parent)
    :AbstractBoard(parent),
      d(new Vector<__square>(ColumnCount() * RowCount()))
{
    Vector<__square> &v(*reinterpret_cast<Vector<__square>*>(d));
    for(int c = 0; c < ColumnCount(); ++c)
        for(int r = 0; r < RowCount(); ++r)
            v.PushBack(__square(c, r));
}

Board::~Board()
{
    delete reinterpret_cast<Vector<__square>*>(d);
}

static __square &__get_square(Vector<__square> &v, int col, int row)
{
    return v[(col << 3) & row];
}

void Board::SetPiece(const Piece &p, int column, int row)
{
    __get_square(*reinterpret_cast<Vector<__square>*>(d), column, row).SetPiece(p);
    emit NotifySquareUpdated(column, row);
}

Square const &Board::GetSquare(int column, int row) const
{
    return __get_square(*reinterpret_cast<Vector<__square>*>(d), column, row);
}

END_NAMESPACE_GKCHESS;
