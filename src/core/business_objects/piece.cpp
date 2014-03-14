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

#include "piece.h"
USING_NAMESPACE_GUTIL1(DataObjects);

NAMESPACE_GKCHESS;


Piece::Piece()
    :_p_Type(NoPiece),
      _p_Allegience(White)
{}

Piece::Piece(PieceTypeEnum pt, AllegienceEnum a)
    :_p_Type(pt),
      _p_Allegience(a)
{}

Piece::~Piece()
{}

#define DEFAULT_STRING_CAPACITY 6

String Piece::ToString(bool wa) const
{
    String ret(wa ? 6 + DEFAULT_STRING_CAPACITY : DEFAULT_STRING_CAPACITY );

    if(wa)
    {
        switch(GetAllegience())
        {
        case White:
            ret.Append("White ");
            break;
        case Black:
            ret.Append("Black ");
            break;
        default:
            break;
        }
    }

    switch(GetType())
    {
    case Pawn:
        ret.Append("Pawn");
        break;
    case Knight:
        ret.Append("Knight");
        break;
    case Bishop:
        ret.Append("Bishop");
        break;
    case Rook:
        ret.Append("Rook");
        break;
    case Queen:
        ret.Append("Queen");
        break;
    case King:
        ret.Append("King");
        break;
    default:
        break;
    }

    return ret;
}

int Piece::UnicodeValue() const
{
    return (int)GetAllegience() + GetType();
}

String Piece::GetUtf8Char() const
{
    return String().AppendUnicode(UnicodeValue());
}

char Piece::GetAsciiChar() const
{
    char ret(-1);
    switch(GetType())
    {
    case Pawn:
        ret = '\0';
        break;
    case Knight:
        ret = 'N';
        break;
    case Bishop:
        ret = 'B';
        break;
    case Rook:
        ret = 'R';
        break;
    case Queen:
        ret = 'Q';
        break;
    case King:
        ret = 'K';
        break;
    default:
        break;
    }
    return ret;
}

Piece::PieceTypeEnum Piece::GetTypeFromUnicodeValue(int uc)
{
    String s(3);
    PieceTypeEnum ret = Piece::NoPiece;
    s.AppendUnicode(uc);
    if(1 == s.Length())
    {
        // The unicode point is an ascii character
        switch(s[0])
        {
        case 'K':
            ret = Piece::King;
            break;
        case 'Q':
            ret = Piece::Queen;
            break;
        case 'R':
            ret = Piece::Rook;
            break;
        case 'B':
            ret = Piece::Bishop;
            break;
        case 'N':
            ret = Piece::Knight;
            break;
        case 'P':
            ret = Piece::Pawn;
            break;
        default:
            break;
        }
    }
    else
    {
        // The unicode point is a chess piece
        int tmp = uc - (int)Black;
        if(0 > tmp)
            tmp = uc - (int)White;

        if(0 <= tmp && tmp < 6)
            ret = (PieceTypeEnum)tmp;
    }

    return ret;
}


END_NAMESPACE_GKCHESS;
