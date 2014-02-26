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

NAMESPACE_GKCHESS;


Board::Board()
{
    for(GINT32 i = 0; i < ColumnCount(); ++i)
    {
        for(GINT32 j = 0; j < RowCount(); ++j)
        {
            m_board[i][j] = Square(i, j);
        }
    }
}

Board::Board(const Board &b)
{
    for(GINT32 i = 0; i < ColumnCount(); ++i)
    {
        for(GINT32 j = 0; j < RowCount(); ++j)
        {
            m_board[i][j] = b.m_board[i][j];
        }
    }
}

void Board::Clear()
{
    for(int i = 0; i < ColumnCount(); ++i)
    {
        for(int j = 0; j < RowCount(); ++j)
        {
            RemovePiece(i, j);
        }
    }
}

void Board::RemovePiece(int column, int row)
{
    Square &s(m_board[column][row]);
    if(s.GetPiece())
        s.SetPiece(0);
}

Square &Board::GetSquare(int column, int row)
{
    return m_board[column][row];
}

Square const &Board::GetSquare(int column, int row) const
{
    return m_board[column][row];
}

void Board::Move(Square &source, Square &dest)
{
    Piece *p = source.GetPiece();
    if(p)
        dest.SetPiece(p);
}

void Board::InitPiece(Piece *p, int column, int row)
{
    m_board[column][row].SetPiece(p);
}

END_NAMESPACE_GKCHESS;
