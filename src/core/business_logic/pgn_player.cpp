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
#include "gutil_file.h"
USING_NAMESPACE_GUTIL1(DataAccess);

NAMESPACE_GKCHESS;


PGN_Player::MoveData::MoveData()
    :PieceType(Piece::Pawn),
      SourceColumn(-1), SourceRow(-1),
      DestColumn(-1), DestRow(-1),
      Captures(false),
      CastleType(NoCastle)
{}


PGN_Player::PGN_Player()
{
}

PGN_Player PGN_Player::FromText(const GUtil::DataObjects::String &pgn_text)
{

}

PGN_Player PGN_Player::FromFile(const GUtil::DataObjects::String &pgn_filename)
{
    File f(pgn_filename);
    f.Open(File::OpenRead);
    return FromText(f.Read());
}


END_NAMESPACE_GKCHESS;
