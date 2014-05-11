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

#include "pgn_movedata.h"
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;


PGN_MoveData::PGN_MoveData()
    :MoveNumber(0),
      PieceMoved(0),
      PiecePromoted(0),
      SourceFile(0),
      SourceRank(0),
      DestFile(0),
      DestRank(0)
{}

static const char *__convert_piece_char_to_name(char c)
{
    switch(c)
    {
    case '\0':
    case 'P':
        return "Pawn";
    case 'K':
        return "King";
    case 'Q':
        return "Queen";
    case 'B':
        return "Bishop";
    case 'N':
        return "Knight";
    case 'R':
        return "Rook";
    case 'A':
        return "Archbishop";
    case 'C':
        return "Chancellor";
    default:
        return "";
    }
}

String PGN_MoveData::ToString() const
{
    String ret;
    if(Flags.TestFlag(CastleHSide))
        ret.Append("O-O");
    else if(Flags.TestFlag(CastleASide))
        ret.Append("O-O-O");
    else
    {
        if(0 != PieceMoved && 'P' != PieceMoved)
            ret.Append(PieceMoved);

        if(0 != SourceFile || 0 != SourceRank)
        {
            if(0 != SourceFile)
                ret.Append(SourceFile);
            if(0 != SourceRank)
                ret.Append(String::FromInt(SourceRank));
        }

        if(Flags.TestFlag(Capture))
            ret.Append('x');

        ret.Append(String::Format("%c%d", DestFile, DestRank));

        if(0 != PiecePromoted)
            ret.Append(String::Format("=%c", PiecePromoted));
    }

    if(Flags.TestFlag(Check))
        ret.Append('+');
    else if(Flags.TestFlag(CheckMate))
        ret.Append('#');

    if(Flags.TestFlag(Blunder))
        ret.Append("??");
    else if(Flags.TestFlag(Mistake))
        ret.Append("?");
    else if(Flags.TestFlag(Dubious))
        ret.Append("?!");
    else if(Flags.TestFlag(Interesting))
        ret.Append("!?");
    else if(Flags.TestFlag(Good))
        ret.Append("!");
    else if(Flags.TestFlag(Brilliant))
        ret.Append("!!");

    if(!Comment.IsEmpty())
        ret.Append(String::Format(" {%s}", Comment.ConstData()));
    return ret;
}

String PGN_MoveData::ToPrettyString() const
{
    String ret(25);
    if(Flags.TestFlag(CastleHSide))
        ret.Append("Castle H-Side");
    else if(Flags.TestFlag(CastleASide))
        ret.Append("Castle A-Side");
    else
    {
        ret.Append(__convert_piece_char_to_name(PieceMoved));

        if(0 != SourceFile || 0 != SourceRank)
        {
            ret.Append(" on ");
            if(0 != SourceFile)
                ret.Append(SourceFile);
            if(0 != SourceRank)
                ret.Append(String::FromInt(SourceRank));
        }

        if(Flags.TestFlag(Capture))
            ret.Append(" takes ");
        else
            ret.Append(" to ");

        ret.Append(String::Format("%c%d", DestFile, DestRank));

        if(0 != PiecePromoted)
            ret.Append(String::Format(" promotes to %s",
                                      __convert_piece_char_to_name(PiecePromoted)));
    }

    if(Flags.TestFlag(Check))
        ret.Append(" check");
    else if(Flags.TestFlag(CheckMate))
        ret.Append(" checkmate");

    if(Flags.TestFlag(Blunder))
        ret.Append(" (blunder)");
    else if(Flags.TestFlag(Mistake))
        ret.Append(" (mistake)");
    else if(Flags.TestFlag(Dubious))
        ret.Append(" (dubious move)");
    else if(Flags.TestFlag(Interesting))
        ret.Append(" (interesting move)");
    else if(Flags.TestFlag(Good))
        ret.Append(" (good move)");
    else if(Flags.TestFlag(Brilliant))
        ret.Append(" (brilliant move)");

    if(!Comment.IsEmpty())
        ret.Append(String::Format(" {%s}", Comment.ConstData()));

    return ret;
}


END_NAMESPACE_GKCHESS;
