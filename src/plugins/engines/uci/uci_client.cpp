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

#include "uci_client.h"
#include "gutil_strings.h"
#include "gkchess_globals.h"
#include "gutil_consolelogger.h"
#include <QFile>
#include <QMutex>
#include <QWaitCondition>
#include <QtConcurrentRun>
#include <QtPlugin>
USING_NAMESPACE_GUTIL;


namespace{

struct d_t
{
    QString engine;
    QStringList arguments;
    bool thinking;

    QProcess process;
    GKChess::UCI_Client::EngineInfo info;


    // This data is protected by the lock and wait condition
    QList<QByteArray> outqueue;
    bool cancel_thread;

    QMutex lock;
    QWaitCondition wc;
    QFuture<void> writer_thread;

    d_t()
        :thinking(false),
          cancel_thread(false)
    {}
};

}


NAMESPACE_GKCHESS;

#define UCI_ANALYSE_MODE_TAG "UCI_AnalyseMode"


static QByteArray __get_next_token(const QByteArray &ba, int indx)
{
    QByteArray ret;
    bool first_chars_seen = false;
    for(int i = indx; i < ba.length(); ++i)
    {
        if(String::IsWhitespace(ba[i])){
            if(first_chars_seen)
                break;
        }
        else
        {
            first_chars_seen = true;
            ret.append(ba[i]);
        }
    }
    return ret;
}

static QByteArray __get_option_name(const QByteArray &ba, int indx, const char *terminator)
{
    QByteArray ret;
    int i = indx;
    int term_len = strlen(terminator);
    while(i < ba.length() &&
          0 != memcmp(terminator, ba.constData() + i, term_len))
    {
        ret.append(ba[i]);
        ++i;
    }
    return ret.trimmed();
}

static QByteArray __get_option_value(const QByteArray &ba, int indx)
{
    QByteArray ret;
    int i = indx;
    while(i < ba.length())
    {
        char c = ba[i];
        if(String::IsWhitespace(c)){
            break;
        }
        else{
            ret.append(c);
        }
        ++i;
    }
    return ret.trimmed();
}

static void __start_and_validate_engine(d_t *d, bool populate_data)
{
    if(!QFile(d->engine).exists())
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "File does not exist");

    d->process.setReadChannel(QProcess::StandardOutput);
    //p.setProcessChannelMode(QProcess::MergedChannels);
    d->process.start(d->engine, d->arguments);
    if(!d->process.waitForStarted())
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Engine unable to start");

    // Ignore the first bit of junk the engine sends
    d->process.waitForReadyRead();
    d->process.readAllStandardOutput();

    GASSERT(d->process.state() == QProcess::Running);

    bool valid = true;
    QString name;

    d->process.write("uci\n");
    d->process.waitForReadyRead();
    QByteArray output = d->process.readAllStandardOutput();

    if(populate_data)
    {
        valid = false;
        int cur_idx = 0;
        while(cur_idx < output.length())
        {
            int nl_idx = output.indexOf('\n', cur_idx);
            QByteArray line;
            if(-1 != nl_idx){
                line = output.left(nl_idx).right(nl_idx - cur_idx);
                cur_idx = nl_idx + 1;
            }

            if(line.startsWith("id"))
            {
                int name_idx = line.indexOf("name");
                int author_idx = line.indexOf("author");

                if(-1 != name_idx){
                    name = line.right(line.length() - (name_idx + 5));
                    d->info.Name = name;
                }
                if(-1 != author_idx){
                    d->info.Author = line.right(line.length() - (author_idx + 7));
                }
            }
            else if(line.startsWith("option"))
            {
                int name_idx = line.indexOf("name");
                int type_idx = line.indexOf("type");

                // Must have a name and type
                if(-1 == name_idx || -1 == type_idx)
                    break;

                QString opt_name = __get_option_name(line, name_idx + 5, "type");
                QString opt_type = __get_option_value(line, type_idx + 5);
                UCI_Client::Option_t *opt = 0;

                if(opt_type == "spin")
                {
                    int default_idx = line.indexOf("default");
                    int min_idx = line.indexOf("min");
                    int max_idx = line.indexOf("max");
                    if(-1 == default_idx || -1 == min_idx || -1 == max_idx)
                        break;

                    QString default_str = __get_option_value(line, default_idx + 8);
                    QString min_str = __get_option_value(line, min_idx + 4);
                    QString max_str = __get_option_value(line, max_idx + 4);
                    opt = new IEngine::SpinOption(opt_name, default_str.toInt(), min_str.toInt(), max_str.toInt());
                }
                else if(opt_type == "string")
                {
                    int default_idx = line.indexOf("default");
                    if(-1 == default_idx)
                        break;

                    QString val = __get_option_value(line, default_idx + 8);
                    opt = new IEngine::StringOption(opt_name, val);

                }
                else if(opt_type == "check")
                {
                    int default_idx = line.indexOf("default");
                    if(-1 == default_idx)
                        break;

                    QString val_str = __get_option_value(line, default_idx + 8);
                    bool checked = val_str.toLower() == "true";
                    opt = new IEngine::CheckOption(opt_name, checked);
                }
                else if(opt_type == "combo")
                {
                    opt = new IEngine::ComboOption(opt_name, QStringList(), QString());
                }
                else if(opt_type == "button")
                {
                    opt = new IEngine::ButtonOption(opt_name);
                }
                else
                {
                    // Unrecognized option
                    break;
                }

                if(opt)
                    d->info.Options.Insert(opt_name, opt);
            }
            else if(line.startsWith("uciok"))
            {
                if(!name.isEmpty())
                    valid = true;
                break;
            }
        }
    }

    if(!valid)
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "The engine is not UCI compatible");

    d->process.write("isready\n");
    d->process.waitForReadyRead();
    char ba[32];
    bool ready_ok = false;

    qint64 len = d->process.readLine(ba, sizeof(ba));
    while(0 < len && !(ready_ok = QByteArray(ba).contains("readyok"))){
        len = d->process.readLine(ba, sizeof(ba));
    }
    if(!ready_ok)
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Engine unable to get ready");
}

static void __writer_thread(d_t *d)
{
    d->lock.lock();
    while(!d->cancel_thread)
    {
        while(!d->cancel_thread && d->outqueue.length() == 0){
            d->wc.wait(&d->lock);
        }

        if(d->process.state() != QProcess::Running)
            continue;

        // Write any data to the engine
        while(0 < d->outqueue.length())
        {
            QByteArray str = d->outqueue.front();
            d->outqueue.pop_front();

            d->lock.unlock();
            {
                // Slow task: Have to write data to the engine
                d->process.write(str + '\n');
            }
            d->lock.lock();
        }
    }
    d->lock.unlock();
}



UCI_Client::UCI_Client(QObject *parent)
    :IEngine(parent)
{
    G_D_INIT();

    connect(this, SIGNAL(BestMove(QByteArray)),
            this, SLOT(_best_move_received()));
}

IEngine *UCI_Client::Create() const
{
    return new UCI_Client(parent());
}

UCI_Client::~UCI_Client()
{
    if(IsEngineStarted())
        StopEngine();
    G_D_UNINIT();
}

void UCI_Client::StartEngine(const QString &path_to_engine, const QStringList &args)
{
    if(IsEngineStarted()){
        GDEBUG("Engine already started!");
        return;
    }

    G_D;

    d->engine = path_to_engine;
    d->arguments = args;

    try{
        __start_and_validate_engine(d, true);
    } catch(...){
        d->engine.clear();
        d->arguments.clear();
        throw;
    }

    connect(&d->process, SIGNAL(readyRead()), this, SLOT(_data_available()));
    connect(&d->process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(_engine_error(QProcess::ProcessError)));
    connect(&d->process, SIGNAL(finished(int)), this, SLOT(_engine_stopped(int)));

    // Start the background worker
    d->writer_thread = QtConcurrent::run(&__writer_thread, d);
}

void UCI_Client::StopEngine()
{
    if(!IsEngineStarted()){
        GDEBUG("Engine already stopped!");
        return;
    }

    G_D;

    // Disconnect all signals from the process
    disconnect(&d->process, SIGNAL(readyReadStandardOutput()), this, SLOT(_data_available()));
    disconnect(&d->process, SIGNAL(error(QProcess::ProcessError)),
               this, SLOT(_engine_error(QProcess::ProcessError)));
    disconnect(&d->process, SIGNAL(finished(int)), this, SLOT(_engine_stopped(int)));

    _append_to_write_queue("quit");

    d->lock.lock();
    d->cancel_thread = true;
    d->wc.wakeOne();
    d->lock.unlock();

    d->writer_thread.waitForFinished();

    // Give the process time to exit gracefully
    if(!d->process.waitForFinished(5000))
    {
        // If it doesn't exit, then kill it
        d->process.kill();
        d->process.waitForFinished();
    }

    d->engine.clear();
    d->arguments.clear();
}

bool UCI_Client::IsEngineStarted() const
{
    G_D;
    return !d->engine.isEmpty();
}

UCI_Client::EngineInfo::~EngineInfo()
{
    for(typename Map<QString, Option_t *>::iterator iter = Options.begin();
        iter != Options.end();
        ++iter)
        delete iter->Value();
}

const UCI_Client::EngineInfo &UCI_Client::GetEngineInfo() const
{
    G_D;
    return d->info;
}

void UCI_Client::_engine_error(QProcess::ProcessError err)
{
    G_D;
    switch(err)
    {
    case QProcess::WriteError:
    case QProcess::ReadError:
    case QProcess::Timedout:
        // kill the process if it's timing out
        d->process.kill();
    default:break;
    }
}

void UCI_Client::_engine_stopped(int ec)
{
    G_D;
    GUTIL_UNUSED(ec);

    // Have to disconnect this while we restart the engine
    disconnect(&d->process, SIGNAL(readyReadStandardOutput()), this, SLOT(_data_available()));

    emit NotifyEngineCrashed();
    __start_and_validate_engine(d, false);

    connect(&d->process, SIGNAL(readyReadStandardOutput()), this, SLOT(_data_available()));

    // Wake up the background thread if it was waiting to do a command
    d->wc.wakeOne();
}

void UCI_Client::_data_available()
{
    G_D;
    QByteArray ba = d->process.readLine();
    while(0 < ba.length())
    {
        // remove the new line at the end (can be CR or LF or any combination)
        if(ba[ba.length() - 1] == (char)0x0a || ba[ba.length() - 1] == (char)0x0d)
            ba.chop(1);
        if(0 < ba.length()){
            if(ba[ba.length() - 1] == (char)0x0a || ba[ba.length() - 1] == (char)0x0d)
                ba.chop(1);
        }

        emit MessageReceived(ba);

        if(ba.contains("bestmove"))
        {
            int indx = ba.indexOf("bestmove");
            QByteArray move = __get_next_token(ba, indx + 9);

            QByteArray ponder;
            indx = ba.indexOf("ponder");
            if(-1 != indx)
                ponder = __get_next_token(ba, indx + 7);

            if(ponder == "(none)")
                ponder = QByteArray();

            emit BestMove(move, ponder);
        }

        ba = d->process.readLine();
    }
}

void UCI_Client::SetPosition(const char *data)
{
    QByteArray ba;
    if(-1 != String(data).IndexOf("startpos"))
        ba = String::Format("position %s", data).ConstData();
    else
        ba = String::Format("position fen %s", data).ConstData();
    _append_to_write_queue(ba);
}

void UCI_Client::SetOption(const QString &name, const QVariant &value)
{
    QString data = QString("setoption name %1 value %2")
            .arg(name)
            .arg(value.toString());
    _append_to_write_queue(data.toUtf8());
}

void UCI_Client::StartThinking(const IEngine::ThinkParams &p)
{
    G_D;
    if(d->thinking)
        return;

    QString str = "go";
    if(-1 == p.MoveTime)
        str.append(" infinite");
    else if(0 < p.MoveTime)
        str.append(QString(" movetime %1").arg(p.MoveTime));

    if(0 < p.Depth)
        str.append(QString(" depth %1").arg(p.Depth));

    if(0 < p.Nodes)
        str.append(QString(" nodes %1").arg(p.Nodes));

    if(0 < p.Mate)
        str.append(QString(" mate %1").arg(p.Mate));

    if(0 < p.SearchMoves.length())
        str.append(QString(" searchmoves %1").arg(p.SearchMoves.join(" ")));

    _append_to_write_queue(str.toUtf8());

    // We raise this when we tell the background thread to start thinking, and lower it
    //  when we receive the best move signal, so we don't need to protect this with a lock
    d->thinking = true;
}

void UCI_Client::StopThinking()
{
    G_D;
    if(!d->thinking)
        return;
    _append_to_write_queue("stop");
}

void UCI_Client::_best_move_received()
{
    G_D;
    d->thinking = false;
}

bool UCI_Client::IsThinking() const
{
    G_D;
    return d->thinking;
}

void UCI_Client::_append_to_write_queue(const QByteArray &ba)
{
    G_D;
    d->lock.lock();
    d->outqueue.append(ba);
    d->lock.unlock();
    d->wc.wakeOne();
}


END_NAMESPACE_GKCHESS;


Q_EXPORT_PLUGIN2(uciEnginePlugin, GKChess::UCI_Client)
