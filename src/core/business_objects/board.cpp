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


Board::Board(GINT32 rows, GINT32 columns)
    :m_board(columns)
{
    if(0 > rows || 0 > columns)
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Rows and Columns must be positive");

    // First create the board
    for(GINT32 i = 0; i < columns; ++i)
    {
        m_board.PushBack(Vector<Square>(rows));
        for(GINT32 j = 0; j < rows; ++j){
            m_board[i].PushBack(Square(((OddOrEven(i) && OddOrEven(j)) || (!OddOrEven(i) && !OddOrEven(j))) ?
                                           Square::Dark : Square::Light));
        }
    }

    // Then wire the squares together so they have spacial awareness
    for(GINT32 i = 0; i < columns; ++i)
    {
        for(GINT32 j = 0; j < rows; ++j)
        {
            if(rows > j + 1)
                m_board[i][j].north = &m_board[i][j + 1];
            if(rows > j + 1 && columns > i + 1)
                m_board[i][j].north_east = &m_board[i + 1][j + 1];
            if(columns > i + 1)
                m_board[i][j].east = &m_board[i + 1][j];
            if(0 <= j - 1 && columns > i + 1)
                m_board[i][j].south_east = &m_board[i + 1][j - 1];
            if(0 <= j - 1)
                m_board[i][j].south = &m_board[i][j - 1];
            if(0 <= j - 1 && 0 <= i - 1)
                m_board[i][j].south_west = &m_board[i - 1][j - 1];
            if(0 <= i - 1)
                m_board[i][j].west = &m_board[i - 1][j];
            if(rows > j + 1 && 0 <= i - 1)
                m_board[i][j].north_west = &m_board[i - 1][j + 1];
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
    for(GUINT32 i = 0; i < GetColumns(); ++i){
        for(GUINT32 j = 0; j < GetRows(); ++j)
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
    for(GUINT32 i = 0; i < GetColumns() && ret; ++i){
        for(GUINT32 j = 0; j < GetRows() && ret; ++j){
            if(m_board[i][j].GetPiece())
                ret = false;
        }
    }
    return ret;
}

GUINT32 Board::GetRows() const
{
    return 0 < m_board.Length() ? m_board[0].Length() : 0;
}

GUINT32 Board::GetColumns() const
{
    return m_board.Length();
}

void Board::Setup(SetupTypeEnum ste)
{
    if(!IsEmpty())
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "You must first clear the board before setting up a new game");

    switch(ste)
    {
    case StandardChess:
        if(8 != GetRows() || 8 != GetColumns())
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "Standard chess can only be played on boards that are 8x8");

        // First set up Pawns:
        for(GUINT32 i = 0; i < GetColumns(); ++i){
            m_board[i][1].SetPiece(new Piece(Piece::White, Piece::Pawn));
            m_board[i][6].SetPiece(new Piece(Piece::Black, Piece::Pawn));
        }

        // Then Rooks:
        m_board[0][0].SetPiece(new Piece(Piece::White, Piece::Rook));
        m_board[7][0].SetPiece(new Piece(Piece::White, Piece::Rook));
        m_board[0][7].SetPiece(new Piece(Piece::Black, Piece::Rook));
        m_board[7][7].SetPiece(new Piece(Piece::Black, Piece::Rook));

        // Then Knights:
        m_board[1][0].SetPiece(new Piece(Piece::White, Piece::Knight));
        m_board[6][0].SetPiece(new Piece(Piece::White, Piece::Knight));
        m_board[1][7].SetPiece(new Piece(Piece::Black, Piece::Knight));
        m_board[6][7].SetPiece(new Piece(Piece::Black, Piece::Knight));

        // Then Bishops:
        m_board[2][0].SetPiece(new Piece(Piece::White, Piece::Bishop));
        m_board[5][0].SetPiece(new Piece(Piece::White, Piece::Bishop));
        m_board[2][7].SetPiece(new Piece(Piece::Black, Piece::Bishop));
        m_board[5][7].SetPiece(new Piece(Piece::Black, Piece::Bishop));

        // Then Queens:
        m_board[3][0].SetPiece(new Piece(Piece::White, Piece::Queen));
        m_board[3][7].SetPiece(new Piece(Piece::Black, Piece::Queen));

        // Then Kings:
        m_board[4][0].SetPiece(new Piece(Piece::White, Piece::King));
        m_board[4][7].SetPiece(new Piece(Piece::Black, Piece::King));

        break;
    default:
        break;
    }
}

Board::Square const &Board::GetSquare(GUINT32 column, GUINT32 row) const
{
    return m_board[column][row];
}


Board::Square::Square(ColorEnum c)
    :north(NULL),
      north_east(NULL),
      east(NULL),
      south_east(NULL),
      south(NULL),
      south_west(NULL),
      west(NULL),
      north_west(NULL),
      _p_Color(c),
      _p_Piece(0)
{}


END_NAMESPACE_GKCHESS;
