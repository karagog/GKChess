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
#include "gutil_file.h"

USING_NAMESPACE_GUTIL1(DataAccess);
USING_NAMESPACE_GUTIL1(DataObjects);
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;


PGN_Player::MoveData::MoveData()
    :PlyNumber(0),
      PieceType(Piece::Pawn),
      MoveType(NormalMove),
      SourceColumn(-1), SourceRow(-1),
      DestColumn(-1), DestRow(-1)
{}


PGN_Player::PGN_Player(GameLogic *g)
    :m_game(g)
{}

void PGN_Player::Clear()
{
    m_moves.Empty();
}

void PGN_Player::LoadFromString(const String &pgn_text)
{
    PGN_Parser p(pgn_text);
    Clear();

    /** \todo parse the move text into our MoveData objects */
}

void PGN_Player::LoadFromFile(const String &pgn_filename)
{
    File f(pgn_filename);
    f.Open(File::OpenRead);
    String s( f.Read() );
    f.Close();
    LoadFromString(s);
}


END_NAMESPACE_GKCHESS;
