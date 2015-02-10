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
#include <gutil/string.h>
#include <gkchess_common.h>
#include <gutil/consolelogger.h>
#include <QFile>
#include <QtPlugin>
#include <QVariant>
USING_NAMESPACE_GUTIL;


namespace{

struct d_t
{
    QString engine;
    QStringList arguments;
    bool thinking;

    QProcess process;
    GKChess::UCI_Client::EngineInfo info;

    d_t()
        :thinking(false)
    {}
};

}



NAMESPACE_GKCHESS;


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

void UCI_Client::_start_and_validate_engine(bool populate_data)
{
    G_D;
    if(!QFile(d->engine).exists())
        throw Exception<>("File does not exist");

    d->process.setReadChannel(QProcess::StandardOutput);
    d->process.start(d->engine, d->arguments);
    if(!d->process.waitForStarted())
        throw Exception<>("Engine unable to start");

    // Ignore the first bit of junk the engine sends
    d->process.waitForReadyRead(-1);
    d->process.readAllStandardOutput();

    if(d->process.state() != QProcess::Running)
        throw Exception<>("Engine crashed immediately after starting");

    bool valid = true;
    QString name;

    d->process.write("uci\n");
    d->process.waitForReadyRead(-1);
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

                if(opt){
                    d->info.OptionNames.append(opt_name);
                    d->info.Options.insert(opt_name, opt);
                }
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
        throw Exception<>("The engine is not UCI compatible");
}



UCI_Client::UCI_Client(QObject *parent)
    :IEngine(parent)
{
    G_D_INIT();

    connect(this, SIGNAL(BestMove(const GenericMove &, const GenericMove &)), this, SLOT(_best_move_received()));
    //qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
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
        _start_and_validate_engine(true);
    } catch(...){
        d->engine.clear();
        d->arguments.clear();
        throw;
    }

    connect(&d->process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(_engine_error(QProcess::ProcessError)));
    connect(&d->process, SIGNAL(finished(int)), this, SLOT(_engine_stopped(int)));

    NewGame();
    _wait_for_ready(-1);

    connect(&d->process, SIGNAL(readyRead()), this, SLOT(_read_from_engine()));
}

void UCI_Client::StopEngine()
{
    if(!IsEngineStarted()){
        GDEBUG("Engine already stopped!");
        return;
    }

    G_D;

    _disconnect_from_process();

    _write_to_engine("quit");

    // Give the process time to exit gracefully
    if(!d->process.waitForFinished(5000))
    {
        // If it doesn't exit, then kill it
        d->process.kill();
        d->process.waitForFinished();
    }
}

void UCI_Client::_disconnect_from_process()
{
    G_D;

    // Disconnect all signals from the process
    disconnect(&d->process, SIGNAL(readyRead()), this, SLOT(_read_from_engine()));
    disconnect(&d->process, SIGNAL(error(QProcess::ProcessError)),
               this, SLOT(_engine_error(QProcess::ProcessError)));
    disconnect(&d->process, SIGNAL(finished(int)), this, SLOT(_engine_stopped(int)));

    d->engine.clear();
    d->arguments.clear();
    d->thinking = false;
}

bool UCI_Client::IsEngineStarted() const
{
    G_D;
    return !d->engine.isEmpty();
}

UCI_Client::EngineInfo::~EngineInfo()
{
    for(Option_t *o : Options)
        delete o;
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

    emit NotifyEngineCrashed();

    QString engine = d->engine;
    QStringList args = d->arguments;

    _disconnect_from_process();
    StartEngine(engine, args);
}

void UCI_Client::_read_from_engine()
{
    G_D;
    QByteArray ba = d->process.readLine();
    while(0 < ba.length())
    {
        _process_line_received(ba);
        ba = d->process.readLine();
    }
}

void UCI_Client::_write_to_engine(const QByteArray &ba)
{
    G_D;
    QByteArray line = ba + '\n';
    qint64 len = d->process.write(line);
    if(len == line.length())
        emit MessageSent(ba);
}

void UCI_Client::_process_line_received(QByteArray &ba)
{
    // Remove the newline
    ba.chop(ba.length() - ba.indexOf('\n', ba.length() - 2));
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

        emit BestMove(GenericMove(move), GenericMove(ponder));
    }
}

bool UCI_Client::WaitForReady(int timeout_ms)
{
    G_D;
    disconnect(&d->process, SIGNAL(readyRead()), this, SLOT(_read_from_engine()));

    bool ret = _wait_for_ready(timeout_ms);

    connect(&d->process, SIGNAL(readyRead()), this, SLOT(_read_from_engine()));
    return ret;
}

bool UCI_Client::_wait_for_ready(int timeout_ms)
{
    G_D;
    bool ret = false;
    _write_to_engine("isready");

    // keep reading until we get the readyok
    while(d->process.waitForReadyRead(timeout_ms)){
        QByteArray ba = d->process.readLine();
        _process_line_received(ba);
        if(ba == "readyok"){
            ret = true;
            break;
        }
    }
    return ret;
}

void UCI_Client::NewGame()
{
    _write_to_engine("ucinewgame");
}

void UCI_Client::SetPosition(const char *data)
{
    QByteArray ba;
    if(GUINT32_MAX != String(data).IndexOf("startpos"))
        ba = String::Format("position %s", data).ConstData();
    else
        ba = String::Format("position fen %s", data).ConstData();
    _write_to_engine(ba);
}

void UCI_Client::SetOption(const QString &name, const QVariant &value)
{
    G_D;
    if(!d->info.Options.contains(name))
        return;

    // Apply the change to our in-memory struct
    IEngine::Option_t *opt = d->info.Options[name];
    switch(opt->GetType())
    {
    case IEngine::Option_t::Check:
    {
        IEngine::CheckOption *co = static_cast<IEngine::CheckOption *>(opt);
        co->Value = value.toInt();
    }
        break;
    case IEngine::Option_t::Spin:
    {
        IEngine::SpinOption *so = static_cast<IEngine::SpinOption *>(opt);
        so->Value = value.toInt();
    }
        break;
    case IEngine::Option_t::String:
    {
        IEngine::StringOption *so = static_cast<IEngine::StringOption *>(opt);
        so->Value = value.toString();
    }
        break;
    case IEngine::Option_t::Combo:
    {
        IEngine::ComboOption *co = static_cast<IEngine::ComboOption *>(opt);
        co->Value = value.toString();
    }
        break;
    default:
        break;
    }

    // Write the change to the engine
    QByteArray data = QString("setoption name %1 value %2")
            .arg(name)
            .arg(value.toString()).toLatin1();
    _write_to_engine(data);
}

void UCI_Client::StartThinking(const IEngine::ThinkParams &p)
{
    G_D;
    if(d->thinking)
        return;

    QByteArray str = "go";
    if(-1 == p.SearchTime)
        str.append(" infinite");
    else if(0 < p.SearchTime)
        str.append(QString(" movetime %1").arg(p.SearchTime));

    if(0 < p.Depth)
        str.append(QString(" depth %1").arg(p.Depth));

    if(0 < p.Nodes)
        str.append(QString(" nodes %1").arg(p.Nodes));

    if(0 < p.Mate)
        str.append(QString(" mate %1").arg(p.Mate));

//    if(0 < p.SearchMoves.size())
//        str.append(QString(" searchmoves %1").arg(p.SearchMoves.join(" ")));

    _write_to_engine(str);

    // We raise this when we tell the background thread to start thinking, and lower it
    //  when we receive the best move signal, so we don't need to protect this with a lock
    d->thinking = true;
}

void UCI_Client::StopThinking()
{
    G_D;
    if(!d->thinking)
        return;
    _write_to_engine("stop");
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


END_NAMESPACE_GKCHESS;
