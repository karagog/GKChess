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

#include "pgn_player.h"
#include "gkchess_pgn_parser.h"
#include "board.h"
USING_NAMESPACE_GUTIL;

namespace{
struct d_t
{
    GKChess::Board *board;
    String pgn_text;
    GKChess::PGN_GameData game_data;
    List<GKChess::MoveData> move_data;

    int move_index;

    d_t()
        :move_index(-1)
    {}
};
}

NAMESPACE_GKCHESS;


PGN_Player::PGN_Player(Board *b)
{
    G_D_INIT();
    G_D;
    d->board = b;
}

PGN_Player::~PGN_Player()
{
    G_D_UNINIT();
}

List<MoveData> const &PGN_Player::GetMoveData() const
{
    G_D;
    return d->move_data;
}

PGN_GameData const &PGN_Player::GetGameData() const
{
    G_D;
    return d->game_data;
}

const String &PGN_Player::GetPGNText() const
{
    G_D;
    return d->pgn_text;
}

const Board &PGN_Player::GetBoard() const
{
    G_D;
    return *d->board;
}

// Converts the pgn movedata to the internal movedata struct and adds it to the list
static void __add_move_data(Board &b, List<MoveData> &l, const List<PGN_MoveData> &pgn_movedata)
{
    G_FOREACH_CONST(const PGN_MoveData &pmd, pgn_movedata)
    {
        MoveData md = b.GenerateMoveData(pmd);

        // Recursively populate any variants
        if(pmd.Variants.Length() > 0)
        {
            __add_move_data(b, md.Variants, pmd.Variants);

            // After the last function returns the board could be in any state, so set it back now
            b.FromFEN(md.Position);
        }

        b.Move(md);
        l.Append(md);
    }
}

void PGN_Player::LoadPGN(const String &s)
{
    G_D;
    List<PGN_GameData> games = PGN_Parser::ParseString(s);
    if(games.Length() > 0)
    {
        List<MoveData> move_data;
        const PGN_GameData &gd = games[0];

        // Set the initial position of the board
        if(gd.Tags.Contains("setup") && gd.Tags.At("setup") == "1"){
            GASSERT(gd.Tags.Contains("fen"));
            d->board->FromFEN(gd.Tags.At("fen"));
        }
        else
            d->board->SetupNewGame(Board::SetupStandardChess);

        // We need to create a list of move data from the pgn data
        __add_move_data(*d->board, move_data, gd.Moves);

        d->pgn_text = s;
        d->game_data = gd;
        d->move_data = move_data;
        d->move_index = move_data.Length() - 1;
    }
}

void PGN_Player::Clear()
{
    G_D;
    d->game_data.clear();
    d->move_data.Clear();
    d->pgn_text.Empty();
}

void PGN_Player::Next()
{
    G_D;
    if(d->move_index < d->move_data.Length() - 1){
        ++(d->move_index);
        d->board->Move(d->move_data[d->move_index]);
    }
}

void PGN_Player::Previous()
{
    G_D;
    if(d->move_index >= 0){
        d->board->FromFEN(d->move_data[d->move_index].Position);
        --(d->move_index);
    }
}

void PGN_Player::First()
{
    G_D;
    if(d->move_data.Length() > 0){
        d->move_index = -1;
        d->board->FromFEN(d->move_data[0].Position);
    }
}

void PGN_Player::Last()
{
    G_D;
    if(d->move_data.Length() > 0){
        d->move_index = d->move_data.Length() - 1;
        d->board->FromFEN(d->move_data[d->move_index].Position);
        d->board->Move(d->move_data[d->move_index]);
    }
}


END_NAMESPACE_GKCHESS;
