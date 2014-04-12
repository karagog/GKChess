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
#include "abstractboard.h"
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;


PGN_Player::PGN_Player(IGameLogic *gl)
    :i_logic(gl)
{}

void PGN_Player::Clear()
{
    m_pgnData.Clear();
}

void PGN_Player::LoadFromString(const String &pgn_text)
{
    m_pgnData = new PGN_Parser::Data_t(PGN_Parser(pgn_text).Data);
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
