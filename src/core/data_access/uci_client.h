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

#include <QThread>

#include "gkchess_globals.h"
#include "gutil_map.h"

namespace GKChess
{


/** A class to handle access to a UCI-compatible chess engine. */
class UCI_Client :
        private QThread
{
    Q_OBJECT
public:

    struct Option_t
    {
        enum TypeEnum
        {
            NoType,
            Spin,
            Check,
            String,
            Button
        };

        QString Name;

        /** Returns the type of this option.  You can safely cast it to the appropriate type. */
        virtual TypeEnum GetType() const{ return NoType; }

        Option_t(const QString &name) :Name(name){}
        virtual ~Option_t(){}
    };

    struct SpinOption : public Option_t
    {
        int Value;
        const int Default;
        const int Min;
        const int Max;

        TypeEnum GetType() const{ return Spin; }

        SpinOption(const QString &name, int val, int min, int max)
            :Option_t(name), Value(val), Default(val), Min(min), Max(max) {}
    };

    struct StringOption : public Option_t
    {
        QString Value;
        const QString Default;

        TypeEnum GetType() const{ return String; }

        StringOption(const QString &name, const QString &s) :Option_t(name), Value(s), Default(s) {}
    };

    struct CheckOption : public Option_t
    {
        bool Value;
        const bool Default;

        TypeEnum GetType() const{ return Check; }

        CheckOption(const QString &name, bool v) :Option_t(name), Value(v), Default(v) {}
    };

    struct ButtonOption : public Option_t
    {
        TypeEnum GetType() const{ return Button; }

        ButtonOption(const QString &name) : Option_t(name) {}
    };

    struct EngineInfo
    {
        QString Name;
        QString Author;
        GUtil::Map<QString, Option_t *> Options;

        ~EngineInfo();
    };

    /** Constructs an instance of the uci client with the given engine.  The engine
     *  must be valid or this will throw an exception.
     *
     *  After the constructor, the engine info member will be populated.
    */
    explicit UCI_Client(const QString &path_to_engine, QObject *parent = 0);

    /** Initializes the engine.  This must be called before using. */
    void Startup();

    const EngineInfo &GetEngineInfo() const{ return m_info; }


signals:

private:

    QString m_engine;
    EngineInfo m_info;

    void run();

};


}

#endif // GKCHESS_UCI_CLIENT_H
