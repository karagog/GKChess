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

#ifndef GKCHESS_PGN_PLAYER_H
#define GKCHESS_PGN_PLAYER_H

#include "gkchess_gamelogic.h"
#include "gutil_strings.h"
#include "gutil_map.h"

NAMESPACE_GKCHESS;


/** Plays a PGN file.
 *  Allows you to load a PGN file and step through it.
*/
class PGN_Player
{
    PGN_Player();

    struct MoveData
    {
        Piece::PieceTypeEnum PieceType;

        int SourceColumn, SourceRow;
        int DestColumn, DestRow;

        bool Captures;

        enum CastleTypeEnum{
            NoCastle,
            Castle,
            CastleQueenSide
        } CastleType;

        GUtil::DataObjects::String Comment;

        MoveData();
    };

    GameLogic m_game;
    GUtil::DataObjects::Vector<MoveData> m_moves;

    GUtil::DataObjects::Map<GUtil::DataObjects::String, GUtil::DataObjects::String> m_tags;

public:

    static PGN_Player FromText(const GUtil::DataObjects::String &pgn_text);
    static PGN_Player FromFile(const GUtil::DataObjects::String &pgn_filename);

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_PGN_PLAYER_H
