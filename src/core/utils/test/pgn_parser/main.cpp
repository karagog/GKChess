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
#include "gkchess_pgn_parser.h"
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GKCHESS;

void _parse_pgn(String const &);

int main(int argc, char *argv[])
{
    if(argc != 2){
        Console::WriteLine("You must pass a single file path to a pgn file.");
    }

    String pgn_file(argv[1]);
    try
    {
        _parse_pgn(pgn_file);
    }
    catch(const Exception<> &ex)
    {
        ConsoleLogger().LogException(ex);
        return -1;
    }

    return 0;
}


void _parse_pgn(const String &s)
{
    Vector<PGN_Parser::GameData> games = PGN_Parser::ParseFile(s);

    if(0 == games.Length()){
        Console::WriteLine("There were no games in that file");
        return;
    }

    // Show the tag pairs we read
    Console::WriteLine("\nShowing tag section contents:");

    Map<String, String> const &tags( games[0].Tags );
    for(typename Map<String, String>::const_iterator iter(tags.begin());
        iter != tags.end();
        ++iter)
    {
        Console::WriteLine(String::Format("Key: \"%s\" \tValue: \"%s\"", iter->Key().ConstData(), iter->Value().ConstData()));
    }

    // Show the moves we read:
    Console::WriteLine("\nShowing move text:");

    Vector<PGN_Parser::MoveData> const &moves( games[0].Moves );
    int cnt = 0;
    for(typename Vector<PGN_Parser::MoveData>::const_iterator iter(moves.begin());
        iter != moves.end();
        ++iter, ++cnt)
    {
        if((0x1 & cnt) == 0)
            Console::WriteLine(String::Format("%d. %s %s", (2+cnt)/2, iter->MoveText.ConstData(), iter->ToString().ConstData()));
        else
            Console::WriteLine(String::Format("  ...%s %s", iter->MoveText.ConstData(), iter->ToString().ConstData()));
    }

    if(games[0].Tags.Contains("Result"))
    {
        if(games[0].Tags["Result"] == "1-0")
            Console::WriteLine("White Wins!");
        else if(games[0].Tags["Result"] == "0-1")
            Console::WriteLine("Black Wins!");
        else if(games[0].Tags["Result"] == "1/2-1/2")
            Console::WriteLine("The game ended in a draw");
    }
}
