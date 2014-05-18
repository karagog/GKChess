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

#ifndef GKCHESS_IENGINE_H
#define GKCHESS_IENGINE_H

#include "gutil_map.h"
#include <QObject>
#include <QStringList>

namespace GKChess
{


/** A class to handle access to a chess engine.  All functions throw an exception on failure. */
class IEngine :
        public QObject
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

    struct ComboOption : public Option_t
    {
        QStringList Values;
        const QString &Default;

        ComboOption(const QString &name, const QStringList &vals, const QString &default_val)
            :Option_t(name), Values(vals), Default(default_val){}
    };

    struct ButtonOption : public Option_t
    {
        TypeEnum GetType() const{ return Button; }

        ButtonOption(const QString &name) : Option_t(name) {}
    };

    /** Information that is returned by the engine when you issue the 'uci' command. */
    struct EngineInfo
    {
        QString Name;
        QString Author;
        GUtil::Map<QString, Option_t *> Options;

        void clear(){ Name.clear(); Author.clear(); Options.Clear(); }

        ~EngineInfo();
    };

    IEngine(QObject *parent = 0) : QObject(parent) {}

    /** Starts the engine at the given path with the given arguments.  */
    virtual void StartEngine(const QString &path_to_engine, const QStringList &args = QStringList()) = 0;

    /** Returns true if the engine has been started. */
    virtual bool IsEngineStarted() const = 0;

    /** Stops the engine (quits), or does nothing if it wasn't started. */
    virtual void StopEngine() = 0;

    /** Returns the info about the engine. */
    virtual const EngineInfo &GetEngineInfo() const = 0;

    /** Sets the option name-value pair.  If the value is null, it will reset to the default. */
    virtual void SetOption(const QString &name, const QVariant &value) = 0;

    /** Sets the position for the engine to work on.
     *  This can be "startpos moves e2e4 e7e5 ..." or a FEN string
    */
    virtual void SetPosition(const char *) = 0;


    struct ThinkParams
    {
        /** If the move time is -1 it will search until you say stop.  If it's 0 it has no time to think of a move. */
        int MoveTime;

        /** Constrains the search depth.  0 means unconstrained. */
        int Depth;

        /** Constrains the number of search nodes. 0 means unconstrained. */
        int Nodes;

        /** Searches for mate in a given number of moves.  0 means unconstrained. */
        int Mate;

        /** Constrains the engine to consider only these moves. An empty list means unconstrained. */
        QStringList SearchMoves;

        ThinkParams()
            :MoveTime(-1),
              Depth(0),
              Nodes(0)
        {}
    };

    /** Tells the engine to start thinking about the best move.  If it is already
     *  thinking, it does nothing.  The engine may stop thinking automatically based
     *  on the ThinkParams, in which case the best move signal will be emitted before
     *  you tell it to stop thinking.
    */
    virtual void StartThinking(const ThinkParams & = ThinkParams()) = 0;

    /** Returns true if the engine is currently thinking. */
    virtual bool IsThinking() const = 0;

    /** Tells the engine to stop thinking and give the best move.
     *  Does nothing if the engine is not thinking.
    */
    virtual void StopThinking() = 0;


signals:

    /** A line has been received from the engine.  This is emitted for every single line
     *  of engine output so you can use it for logging or whatever
    */
    void MessageReceived(const QByteArray &);

    /** This signal is emitted whenever a 'bestmove' is received from the engine.
     *  \param move The move text
     *  \param ponder The move that the engine would like to ponder
    */
    void BestMove(const QByteArray &move, const QByteArray &ponder = QByteArray());

    /** This signal notifies that the engine has crashed. */
    void NotifyEngineCrashed();

};


}


Q_DECLARE_INTERFACE(GKChess::IEngine, "GKChess.IEngine")

#endif // GKCHESS_IENGINE_H
