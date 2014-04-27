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


PGN_Player::PGN_Player(Board *b)
    :m_board(b)
{}

void PGN_Player::SetGameData(const PGN_Parser::GameData &gm)
{
    m_pgnData = new PGN_Parser::GameData(gm);
}

void PGN_Player::Clear()
{
    m_pgnData.Clear();
}

void PGN_Player::Next()
{

}

void PGN_Player::Previous()
{

}


END_NAMESPACE_GKCHESS;
