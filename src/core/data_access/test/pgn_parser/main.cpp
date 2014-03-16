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
#include "gutil_file.h"
#include "gkchess_pgn_parser.h"
USING_NAMESPACE_GUTIL1(DataAccess);
USING_NAMESPACE_GUTIL1(DataObjects);
USING_NAMESPACE_GUTIL1(Logging);
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GKCHESS;

void _parse_pgn(String const &);

int main(int argc, char *argv[])
{
    if(argc != 2){
        Console::WriteLine("You must pass a single file path to a pgn file.");
    }

    try
    {
        String pgn_text;
        File f(argv[1]);
        f.Open(File::OpenRead);
        pgn_text = f.Read();
        f.Close();

        _parse_pgn(pgn_text);
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
    PGN_Parser pgn(s);

    // Show the tag pairs we read
    Console::WriteLine("\nShowing tag section contents:");

    Map<String, String> const &tags( pgn.GetData().Tags );
    for(typename Map<String, String>::const_iterator iter(tags.begin());
        iter != tags.end();
        ++iter)
    {
        Console::WriteLine(String::Format("Key: \"%s\" \tValue: \"%s\"", iter->Key().ConstData(), iter->Value().ConstData()));
    }

    // Show the moves we read:
    Console::WriteLine("\nShowing move text:");

    Vector<PGN_MoveData> const &moves( pgn.GetData().Moves );
    int cnt = 0;
    for(typename Vector<PGN_MoveData>::const_iterator iter(moves.begin());
        iter != moves.end();
        ++iter, ++cnt)
    {
        if((0x1 & cnt) == 0)
            Console::WriteLine(String::Format("%d. %s %s", (2+cnt)/2, iter->Text.ConstData(), iter->ToString().ConstData()));
        else
            Console::WriteLine(String::Format("  ...%s %s", iter->Text.ConstData(), iter->ToString().ConstData()));
    }

    if(1 == pgn.GetData().Result)
        Console::WriteLine("White Wins!");
    else if(-1 == pgn.GetData().Result)
        Console::WriteLine("Black Wins!");
    else if(0 == pgn.GetData().Result)
        Console::WriteLine("The game ended in a draw");
}
