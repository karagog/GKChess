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

#include "gkchess_uci_client.h"
#include <QCoreApplication>
#include "gutil_consolelogger.h"
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GUTIL;

int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);
    UCI_Client *cli;

    try
    {
         cli = new UCI_Client("/usr/games/stockfish");
    }
    catch(const Exception<> &ex)
    {
        ConsoleLogger().LogException(ex);
        return -1;
    }

    Console::WriteLine("Engine info:");
    Console::WriteLine(String::Format("Name: %s", cli->GetEngineInfo().Name.toUtf8().constData()));
    Console::WriteLine(String::Format("Author: %s", cli->GetEngineInfo().Author.toUtf8().constData()));
    Console::WriteLine();

    for(typename Map<QString, UCI_Client::Option_t *>::const_iterator iter = cli->GetEngineInfo().Options.begin();
        iter != cli->GetEngineInfo().Options.end();
        ++iter)
    {
        switch(iter->Value()->GetType())
        {
        case UCI_Client::Option_t::Spin:
        {
            UCI_Client::SpinOption *opt = dynamic_cast<UCI_Client::SpinOption *>(iter->Value());
            Console::WriteLine(String::Format("Spin option '%s': min,max,default=%d,%d,%d",
                                              opt->Name.toUtf8().constData(),
                                              opt->Min,
                                              opt->Max,
                                              opt->Default));
        }
            break;
        case UCI_Client::Option_t::Check:
        {
            UCI_Client::CheckOption *opt = dynamic_cast<UCI_Client::CheckOption *>(iter->Value());
            Console::WriteLine(String::Format("Check option '%s': default=%d",
                                              opt->Name.toUtf8().constData(),
                                              opt->Default));
        }
            break;
        case UCI_Client::Option_t::String:
        {
            UCI_Client::StringOption *opt = dynamic_cast<UCI_Client::StringOption *>(iter->Value());
            Console::WriteLine(String::Format("String option '%s': default=%s",
                                              opt->Name.toUtf8().constData(),
                                              opt->Default.toUtf8().constData()));
        }
            break;
        case UCI_Client::Option_t::Button:
        {
            UCI_Client::ButtonOption *opt = dynamic_cast<UCI_Client::ButtonOption *>(iter->Value());
            Console::WriteLine(String::Format("Button option '%s'",
                                              opt->Name.toUtf8().constData()));
        }
            break;
        }
    }

    return 0;

//    return a.exec();
}
