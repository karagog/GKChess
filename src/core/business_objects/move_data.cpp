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
     SourceFile(0), SourceRank(0), DestFile(0), DestRank(0)
{}

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

        if(0 != SourceFile){
            ret.Append(String::Format(" on %c", SourceFile));
            if(0 != SourceRank)
                ret.Append(String::FromInt(SourceRank));
        }

        if(Flags.TestFlag(Capture))
            ret.Append(" takes ");
        else
            ret.Append(" to ");

        ret.Append(String::Format("%c%d", DestFile, DestRank));

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
