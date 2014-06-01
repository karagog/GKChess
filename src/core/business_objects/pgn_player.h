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

#include "gutil_strings.h"
#include "gutil_map.h"
#include "gutil_flags.h"
#include "gutil_smartpointer.h"
#include "gkchess_pgn_parser.h"
#include "gkchess_board_movedata.h"

NAMESPACE_GKCHESS;

class Board;


/** Plays a PGN file.
 *  Allows you to load a PGN file and step through it.
*/
class PGN_Player
{
    GKChess::Board &board;
    GUtil::String pgn_text;
    GKChess::PGN_GameData game_data;
    GUtil::List<GKChess::MoveData> move_data;
    int move_index;
public:

    /** Constructs a PGN player with the given game logic.  It will not take ownership. */
    PGN_Player(Board &);

    /** Loads the pgn string. */
    void LoadPGN(const GUtil::String &);

    /** Returns move data used by the player. */
    GUtil::List<MoveData> const &GetMoveData() const;

    /** Returns the pgn game data used by the player. This stores more meta-data about the game, like
     *  who was playing at what event, and so on.
    */
    PGN_GameData const &GetGameData() const;

    /** Returns the pgn text that has been loaded. */
    const GUtil::String &GetPGNText() const;

    /** Unloads the PGN data.  After calling this, "IsLoaded()" returns false. */
    void Clear();

    /** After loading a PGN string, you can step through the moves with this. */
    void Next();

    /** After loading a PGN string, you can step back through the moves with this. */
    void Previous();

    /** After loading a PGN string, you can jump to the first move with this. */
    void First();

    /** After loading a PGN string, you can jump to the last move with this. */
    void Last();

    /** Returns the game board object used by the PGN player. */
    const Board &GetBoard() const;

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_PGN_PLAYER_H
