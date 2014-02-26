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
#include "piece.h"

NAMESPACE_GKCHESS;

Square::Square()
{}

Square::Square(int col, int row)
    :_p_Column(col),
      _p_Row(row),
      _p_EnPassantAvailable(false),
      _p_CastleAvailable(false)
{}

Square::Square(const Square &o)
    :piece(o.piece),
      _p_Column(o._p_Column),
      _p_Row(o._p_Row),
      _p_EnPassantAvailable(o._p_EnPassantAvailable),
      _p_CastleAvailable(o._p_CastleAvailable)
{}

Square &Square::operator = (const Square &o)
{
    piece = o.piece;
    _p_Column = o._p_Column;
    _p_Row = o._p_Row;
    _p_EnPassantAvailable = o._p_EnPassantAvailable;
    _p_CastleAvailable = o._p_CastleAvailable;
    return *this;
}

Square::~Square()
{}

bool Square::operator == (const Square &other)
{
    return GetColumn() == other.GetColumn() &&
            GetRow() == other.GetRow();
}

bool Square::operator != (const Square &other)
{
    return GetColumn() != other.GetColumn() ||
            GetRow() != other.GetRow();
}

void Square::SetPiece(Piece *p)
{
    piece = p;
}


END_NAMESPACE_GKCHESS;
