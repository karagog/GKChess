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
public:

    PGN_Player(GameLogic *);

    void LoadFromString(const GUtil::DataObjects::String &);
    void LoadFromFile(const GUtil::DataObjects::String &);

    /** Returns the tag pairs. */
    const GUtil::DataObjects::Map<GUtil::DataObjects::String, GUtil::DataObjects::String> GetTags() const
    { return m_tags; }

    void Clear();


    /** Describes all the data we need to remember each move. */
    struct MoveData
    {
        int PlyNumber;
        Piece::PieceTypeEnum PieceType;

        int SourceColumn, SourceRow;
        int DestColumn, DestRow;

        bool Captures;

        enum CastleTypeEnum{
            NoCastle,
            Castle,
            CastleQueenSide
        } CastleType;

        GUtil::DataObjects::String MoveText;
        GUtil::DataObjects::String Comment;

        /** Returns true if this move was white's. */
        bool IsWhitesMove() const{ return 0x1 & PlyNumber; }

        MoveData();
    };

    /** Returns the moves from the file. */
    const GUtil::DataObjects::Vector<MoveData> GetMoves() const{ return m_moves; }


private:

    GameLogic *m_game;
    GUtil::DataObjects::Vector<MoveData> m_moves;

    GUtil::DataObjects::Map<GUtil::DataObjects::String, GUtil::DataObjects::String> m_tags;

    /** Populates the heading tags and returns an iterator to the start of the move data section. */
    typename GUtil::DataObjects::String::UTF8ConstIterator
        _parse_heading(const GUtil::DataObjects::String &);

    void _parse_moves(const GUtil::DataObjects::String &);

    void _parse_move(const GUtil::DataObjects::String &, MoveData &);

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_PGN_PLAYER_H
