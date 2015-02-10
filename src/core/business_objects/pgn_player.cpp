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

QList<MoveData> const &PGN_Player::GetMoveData() const
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
static void __add_move_data(Board &b, QList<MoveData> &l, const QList<PGN_MoveData> &pgn_movedata)
{
    for(const PGN_MoveData &pmd : pgn_movedata)
    {
        MoveData md = b.GenerateMoveData(pmd);

        // Recursively populate any variants
        if(pmd.Variants.size() > 0)
        {
            md.Variants.append(QList<MoveData>());
            __add_move_data(b, md.Variants.back(), pmd.Variants);

            // After the last function returns the board could be in any state, so set it back now
            b.FromFEN(md.Position);
        }

        b.Move(md);
        l.append(md);
    }
}

void PGN_Player::LoadPGN(const String &s)
{
    QList<PGN_GameData> games = PGN_Parser::ParseString(s);
    if(games.size() > 0)
    {
        QList<MoveData> tmp_move_data;
        const PGN_GameData &gd = games[0];

        // Set the initial position of the board
        if(gd.Tags.contains("setup") && gd.Tags["setup"] == "1"){
            if(!gd.Tags.contains("fen"))
                throw Exception<>("Setup declared without accompanying FEN");
            board.FromFEN(gd.Tags["fen"]);
        }
        else
            board.SetupNewGame(Board::SetupStandardChess);

        // We need to create a list of move data from the pgn data
        __add_move_data(board, tmp_move_data, gd.Moves);

        pgn_text = s;
        game_data = gd;
        move_data = tmp_move_data;
        move_index = tmp_move_data.size() - 1;
    }
}

void PGN_Player::Clear()
{
    game_data.clear();
    move_data.clear();
    pgn_text.Empty();
}

void PGN_Player::Next()
{
    if(move_index < (int)(move_data.size() - 1)){
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
    if(move_data.size() > 0){
        move_index = -1;
        board.FromFEN(move_data[0].Position);
    }
}

void PGN_Player::Last()
{
    if(move_data.size() > 0){
        move_index = move_data.size() - 1;
        board.FromFEN(move_data[move_index].Position);
        board.Move(move_data[move_index]);
    }
}


END_NAMESPACE_GKCHESS;
