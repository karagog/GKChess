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
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;


Piece::Piece()
    :_p_Type(NoPiece),
      _p_Allegience(AnyAllegience)
{}

Piece::Piece(PieceTypeEnum pt, AllegienceEnum a)
    :_p_Type(pt),
      _p_Allegience(a)
{}

Piece Piece::FromFEN(char c)
{
    Piece::AllegienceEnum a = String::IsUpper(c) ? Piece::White : Piece::Black;
    char tmps[2] = {c, '\0'};
    String::ToUpper(&c, tmps);
    return Piece(GetTypeFromUnicodeValue(c), a);
}

#define DEFAULT_STRING_CAPACITY 6

String Piece::TypeToString(PieceTypeEnum pt)
{
    String ret(DEFAULT_STRING_CAPACITY);
    switch(pt)
    {
    case Pawn:
        ret = "Pawn";
        break;
    case Knight:
        ret = "Knight";
        break;
    case Bishop:
        ret = "Bishop";
        break;
    case Rook:
        ret = "Rook";
        break;
    case Queen:
        ret = "Queen";
        break;
    case King:
        ret = "King";
        break;
    case Archbishop:
        ret = "Archbishop";
        break;
    case Chancellor:
        ret = "Chancellor";
        break;
    default:
        break;
    }
    return ret;
}

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
    ret.Append(TypeToString(GetType()));
    return ret;
}

int Piece::UnicodeValue() const
{
    int ret = -1;
    if(0 <= GetAllegience() && GetAllegience() < 2 &&
       0 <= GetType() && GetType() < 6)
       ret = 0x2654 + 6*GetAllegience() + GetType();
    return ret;
}

String Piece::GetUtf8Char() const
{
    return String().AppendUnicode(UnicodeValue());
}

char Piece::ToFEN() const
{
    char ret(0);
    switch(GetType())
    {
    case Pawn:
        ret = White == GetAllegience() ? 'P' : 'p';
        break;
    case Knight:
        ret = White == GetAllegience() ? 'N' : 'n';
        break;
    case Bishop:
        ret = White == GetAllegience() ? 'B' : 'b';
        break;
    case Rook:
        ret = White == GetAllegience() ? 'R' : 'r';
        break;
    case Queen:
        ret = White == GetAllegience() ? 'Q' : 'q';
        break;
    case King:
        ret = White == GetAllegience() ? 'K' : 'k';
        break;
    case Archbishop:
        ret = White == GetAllegience() ? 'A' : 'a';
        break;
    case Chancellor:
        ret = White == GetAllegience() ? 'C' : 'c';
        break;
    default:
        break;
    }
    return ret;
}

char const *Piece::ToPGN() const
{
    return ToPGN(GetType());
}

char const *Piece::ToPGN(Piece::PieceTypeEnum p)
{
    char const *ret(0);
    switch(p)
    {
    case Pawn:
        ret = "";
        break;
    case Knight:
        ret = "N";
        break;
    case Bishop:
        ret = "B";
        break;
    case Rook:
        ret = "R";
        break;
    case Queen:
        ret = "Q";
        break;
    case King:
        ret = "K";
        break;
    case Archbishop:
        ret = "A";
        break;
    case Chancellor:
        ret = "C";
        break;
    default:
        break;
    }
    return ret;
}

Piece::PieceTypeEnum Piece::GetTypeFromPGN(char c)
{
    PieceTypeEnum ret = NoPiece;
    switch(c)
    {
    case '\0':
    case 'P':
        ret = Piece::Pawn;
        break;
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
    case 'A':
        ret = Piece::Archbishop;
        break;
    case 'C':
        ret = Piece::Chancellor;
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
    if(1 < s.Length())
    {
        THROW_NEW_GUTIL_EXCEPTION(NotImplementedException);
        // The unicode point is a chess piece
//        int tmp = uc - (int)Black;
//        if(0 > tmp)
//            tmp = uc - (int)White;

//        if(0 <= tmp && tmp < 6)
//            ret = (PieceTypeEnum)tmp;
    }
    else if(1 == s.Length() || (s.IsEmpty() && !s.IsNull()))
    {
        ret = GetTypeFromPGN(s[0]);
    }
    return ret;
}


END_NAMESPACE_GKCHESS;
