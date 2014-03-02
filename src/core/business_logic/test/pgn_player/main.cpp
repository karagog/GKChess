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

#include "gutil_console.h"
#include "gutil_consolelogger.h"
#include "gutil_strings.h"
#include "gkchess_pgn_player.h"
#include "gkchess_gamelogic.h"
USING_NAMESPACE_GUTIL1(DataAccess);
USING_NAMESPACE_GUTIL1(DataObjects);
USING_NAMESPACE_GUTIL1(Logging);
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GKCHESS;

#define PGN_FILENAME "../../sample_pgn/"

int main(int argc, char *argv[])
{
    if(argc != 2){
        Console::WriteLine("You must pass a single file path to a pgn file.");
    }

    GameLogic logic;
    PGN_Player pgn(&logic);

    try
    {
        pgn.LoadFromFile(argv[1]);
    }
    catch(const Exception<> &ex)
    {
        ConsoleLogger().LogException(ex);
        return -1;
    }

    // Show the tag pairs we read
    Console::WriteLine("\nShowing tag section contents:");

    Map<String, String> const &tags( pgn.GetTags() );
    for(typename Map<String, String>::const_iterator iter(tags.begin());
        iter != tags.end();
        ++iter)
    {
        Console::WriteLine(String::Format("Key: \"%s\" \tValue: \"%s\"", iter->Key().ConstData(), iter->Value().ConstData()));
    }

    // Show the moves we read:
    Console::WriteLine("\nShowing move text:");

    Vector<PGN_Player::MoveData> const &moves( pgn.GetMoves() );
    for(typename Vector<PGN_Player::MoveData>::const_iterator iter(moves.begin());
        iter != moves.end();
        ++iter)
    {
        if(iter->IsWhitesMove())
            Console::Write("W: ");
        else
            Console::Write("B: ");

        Console::WriteLine(String::Format("%d. %s", iter->PlyNumber, iter->MoveText.ConstData()));
    }


    return 0;
}
