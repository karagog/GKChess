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

NAMESPACE_GKCHESS;


Square::Square(Board *b, ColorEnum c)
    :board(b),
      north(NULL),
      north_east(NULL),
      east(NULL),
      south_east(NULL),
      south(NULL),
      south_west(NULL),
      west(NULL),
      north_west(NULL),
      en_passant(false),
      castle_available(false),
      _p_Color(c),
      _p_Piece(0),
      _p_Column(-1),
      _p_Row(-1)
{}

Square::~Square()
{}

Square *Square::GetSquare(Square::RelativeSquareEnum r)
{
    return const_cast<Square *>(
                const_cast<Square const *>(this)->GetSquare(r)
           );
}

Square const *Square::GetSquare(Square::RelativeSquareEnum r) const
{
    Square const *ret(NULL);
    switch(r)
    {
    case North:
        ret = north;
        break;
    case NorthEast:
        ret = north_east;
        break;
    case East:
        ret = east;
        break;
    case SouthEast:
        ret = south_east;
        break;
    case South:
        ret = south;
        break;
    case SouthWest:
        ret = south_west;
        break;
    case West:
        ret = west;
        break;
    case NorthWest:
        ret = north_west;
        break;
    default:
        break;
    }
    return ret;
}

bool Square::operator == (const Square &other)
{
    // Two squares that share the same north square must be the same.
    //  If the north square is off the board, then check the south
    return board == other.board &&
        north ? north == other.north : south == other.south;
}

bool Square::operator != (const Square &other)
{
    return board != other.board ||
        north ? north != other.north : south != other.south;
}


END_NAMESPACE_GKCHESS;
