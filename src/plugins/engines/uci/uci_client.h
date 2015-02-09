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

#ifndef GKCHESS_UCI_CLIENT_H
#define GKCHESS_UCI_CLIENT_H

#include "gkchess_iengine.h"
#include <QProcess>

namespace GKChess{


/** A class to handle access to a UCI-compatible chess engine. */
class UCI_Client :
        public IEngine
{
    Q_OBJECT
    Q_INTERFACES(GKChess::IEngine)
    Q_PLUGIN_METADATA(IID "GKChess.UCI_Engine_Client")
    void *d;
public:

    /** Constructs an instance of the uci client with the given engine.  The engine
     *  must be valid or this will throw an exception.
     *
     *  After the constructor, the engine info member will be populated.
    */
    UCI_Client(QObject *parent = 0);
    ~UCI_Client();

    IEngine *Create() const;

    void StartEngine(const QString &, const QStringList &);
    bool IsEngineStarted() const;
    void StopEngine();

    const EngineInfo &GetEngineInfo() const;
    void SetPosition(const char *);
    void SetOption(const QString &, const QVariant &);
    bool WaitForReady(int);
    void NewGame();

    void StartThinking(const ThinkParams &);
    bool IsThinking() const;
    void StopThinking();


private slots:

    void _read_from_engine();

    void _best_move_received();

    void _engine_error(QProcess::ProcessError err);
    void _engine_stopped(int);


private:

    void _start_and_validate_engine(bool populate_data);

    void _write_to_engine(const QByteArray &);
    void _process_line_received(QByteArray &);

    bool _wait_for_ready(int);
    void _disconnect_from_process();

};


}

#endif // GKCHESS_UCI_CLIENT_H
