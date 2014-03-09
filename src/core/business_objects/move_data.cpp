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

#include "move_data.h"
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(DataObjects);

NAMESPACE_GKCHESS;


MoveData::MoveData()
    :PieceMoved(Piece::Pawn), PiecePromoted(Piece::Pawn),
     SourceFile(-1), SourceRank(-1), DestFile(-1), DestRank(-1)
{}

/** Converts an array index to a string. */
static char const *__index_to_string(int indx)
{
    switch(indx)
    {
    case 0:
        return "a";
    case 1:
        return "b";
    case 2:
        return "c";
    case 3:
        return "d";
    case 4:
        return "e";
    case 5:
        return "f";
    case 6:
        return "g";
    case 7:
        return "h";
    default:
        return 0;
    }
}

String MoveData::ToString() const
{
    String ret(25);
    if(Flags.TestFlag(CastleNormal))
        ret.Append("Castle");
    else if(Flags.TestFlag(CastleQueenSide))
        ret.Append("Castle Queenside");
    else
    {
        ret.Append(Piece(PieceMoved).ToString(false));

        if(-1 != SourceFile){
            ret.Append(String::Format(" on %s", __index_to_string(SourceFile)));
            if(-1 != SourceRank)
                ret.Append(String::FromInt(SourceRank + 1));
        }

        if(Flags.TestFlag(Capture))
            ret.Append(" takes ");
        else
            ret.Append(" to ");

        ret.Append(String::Format("%s%d", __index_to_string(DestFile), DestRank + 1));

        if(Piece::Pawn != PiecePromoted)
            ret.Append(String::Format(" promotes to %s",
                                      Piece(PiecePromoted).ToString(false).ConstData()));
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

    return ret;
}


END_NAMESPACE_GKCHESS;
