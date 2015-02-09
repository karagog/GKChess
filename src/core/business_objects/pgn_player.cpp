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
#include "board.h"
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;


PGN_Player::PGN_Player(Board &b)
    :board(b)
{}

List<MoveData> const &PGN_Player::GetMoveData() const
{
    return move_data;
}

PGN_GameData const &PGN_Player::GetGameData() const
{
    return game_data;
}

const String &PGN_Player::GetPGNText() const
{
    return pgn_text;
}

const Board &PGN_Player::GetBoard() const
{
    return board;
}

// Converts the pgn movedata to the internal movedata struct and adds it to the list
static void __add_move_data(Board &b, List<MoveData> &l, const List<PGN_MoveData> &pgn_movedata)
{
    for(const PGN_MoveData &pmd : pgn_movedata)
    {
        MoveData md = b.GenerateMoveData(pmd);

        // Recursively populate any variants
        if(pmd.Variants.Length() > 0)
        {
            md.Variants.Append(List<MoveData>());
            __add_move_data(b, md.Variants.Back(), pmd.Variants);

            // After the last function returns the board could be in any state, so set it back now
            b.FromFEN(md.Position);
        }

        b.Move(md);
        l.Append(md);
    }
}

void PGN_Player::LoadPGN(const String &s)
{
    List<PGN_GameData> games = PGN_Parser::ParseString(s);
    if(games.Length() > 0)
    {
        List<MoveData> tmp_move_data;
        const PGN_GameData &gd = games[0];

        // Set the initial position of the board
        if(gd.Tags.Contains("setup") && gd.Tags.At("setup") == "1"){
            GASSERT(gd.Tags.Contains("fen"));
            board.FromFEN(gd.Tags.At("fen"));
        }
        else
            board.SetupNewGame(Board::SetupStandardChess);

        // We need to create a list of move data from the pgn data
        __add_move_data(board, tmp_move_data, gd.Moves);

        pgn_text = s;
        game_data = gd;
        move_data = tmp_move_data;
        move_index = tmp_move_data.Length() - 1;
    }
}

void PGN_Player::Clear()
{
    game_data.clear();
    move_data.Clear();
    pgn_text.Empty();
}

void PGN_Player::Next()
{
    if(move_index < (int)(move_data.Length() - 1)){
        ++(move_index);
        board.Move(move_data[move_index]);
    }
}

void PGN_Player::Previous()
{
    if(move_index >= 0){
        board.FromFEN(move_data[move_index].Position);
        --(move_index);
    }
}

void PGN_Player::First()
{
    if(move_data.Length() > 0){
        move_index = -1;
        board.FromFEN(move_data[0].Position);
    }
}

void PGN_Player::Last()
{
    if(move_data.Length() > 0){
        move_index = move_data.Length() - 1;
        board.FromFEN(move_data[move_index].Position);
        board.Move(move_data[move_index]);
    }
}


END_NAMESPACE_GKCHESS;
