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

#include "square.h"
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;


Square::Square()
    :m_column(-1), m_row(-1),
      m_threatsWhite(0), m_threatsBlack(0)
{}

Square::Square(GUINT8 c, GUINT8 r)
    :m_column(c),
      m_row(r),
      m_threatsWhite(-1),
      m_threatsBlack(-1)
{}

Square::Square(const Square &o)
    :m_column(o.m_column), m_row(o.m_row),
      m_threatsWhite(o.m_threatsWhite), m_threatsBlack(o.m_threatsBlack),
      m_piece(o.m_piece)
{}

Square &Square::operator = (const Square &o)
{
    m_column = o.m_column;
    m_row = o.m_row;
    m_piece = o.m_piece;
    m_threatsWhite = o.m_threatsWhite;
    m_threatsBlack = o.m_threatsBlack;
    return *this;
}

bool Square::IsNull() const
{
    return -1 == m_column && -1 == m_row;
}

int Square::GetColumn() const
{
    return m_column;
}

int Square::GetRow() const
{
    return m_row;
}

Piece const &Square::GetPiece() const
{
    return m_piece;
}

void Square::SetPiece(Piece const &p)
{
    m_piece = p;
}

int Square::GetThreatCount(Piece::AllegienceEnum a) const
{
    return Piece::White == a ? m_threatsWhite : m_threatsBlack;
}

void Square::SetThreatCount(Piece::AllegienceEnum a, int c)
{
    if(Piece::White == a)
        m_threatsWhite = c;
    else
        m_threatsBlack = c;
}

bool Square::IsDarkSquare() const
{
    return (0x1 & GetRow()) == (0x1 & GetColumn());
}

String Square::ToString() const
{
    char tmp[2];
    tmp[0] = 'a' + GetColumn();
    tmp[1] = '1' + GetRow();
    return String(tmp, 2);
}

bool Square::operator == (const Square &o) const
{
    return GetColumn() == o.GetColumn() &&
            GetRow() == o.GetRow();
}

bool Square::operator != (const Square &o) const
{
    return GetColumn() != o.GetColumn() ||
            GetRow() != o.GetRow();
}


END_NAMESPACE_GKCHESS;
