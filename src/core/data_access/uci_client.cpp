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
#include "gutil_exception.h"
#include "gutil_strings.h"
#include "gkchess_globals.h"
#include "gutil_consolelogger.h"
#include <QProcess>
#include <QFile>
#include <QMutex>
#include <QWaitCondition>
#include <QtConcurrentRun>
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;

#define UCI_ANALYSE_MODE_TAG "UCI_AnalyseMode"


struct d_t
{
    QString engine;
    QProcess process;
    UCI_Client::EngineInfo info;
    bool debug_output_enabled;
    bool analysis_mode;

    // This data is protected by the lock and wait condition
    QList<QByteArray> outqueue;
    bool cancel_thread;

    QMutex lock;
    QWaitCondition wc;
    QFuture<void> writer_thread;

    d_t()
        :debug_output_enabled(false),
          analysis_mode(false),
          cancel_thread(false)
    {}
};


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

static void __start_and_validate_engine(d_t *d)
{
    if(!QFile(d->engine).exists())
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "File does not exist");

    d->process.setReadChannel(QProcess::StandardOutput);
    //p.setProcessChannelMode(QProcess::MergedChannels);
    d->process.start(d->engine);
    if(!d->process.waitForStarted())
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Engine unable to start");

    // Ignore the first bit of junk the engine sends
    d->process.waitForReadyRead();
    d->process.readAllStandardOutput();

    GASSERT(d->process.state() == QProcess::Running);

    bool valid = false;
    QString name;

    d->process.write("uci\n");
    d->process.waitForReadyRead();
    QByteArray output = d->process.readAllStandardOutput();

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
                opt = new UCI_Client::SpinOption(opt_name, default_str.toInt(), min_str.toInt(), max_str.toInt());
            }
            else if(opt_type == "string")
            {
                int default_idx = line.indexOf("default");
                if(-1 == default_idx)
                    break;

                QString val = __get_option_value(line, default_idx + 8);
                opt = new UCI_Client::StringOption(opt_name, val);

            }
            else if(opt_type == "check")
            {
                int default_idx = line.indexOf("default");
                if(-1 == default_idx)
                    break;

                QString val_str = __get_option_value(line, default_idx + 8);
                bool checked = val_str.toLower() == "true";
                opt = new UCI_Client::CheckOption(opt_name, checked);

                if(opt_name.toLower() == QString(UCI_ANALYSE_MODE_TAG).toLower() && checked)
                    d->analysis_mode = true;
            }
            else if(opt_type == "combo")
            {
                opt = new UCI_Client::ComboOption(opt_name, QStringList(), QString());
            }
            else if(opt_type == "button")
            {
                opt = new UCI_Client::ButtonOption(opt_name);
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

    if(!valid)
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "The engine is not UCI compatible");
}

UCI_Client::UCI_Client(const QString &path_to_engine, QObject *parent)
    :QObject(parent)
{
    G_D_INIT();
    G_D;

    d->engine = path_to_engine;

    // Initialize the engine; this class is useless without a working engine
    __start_and_validate_engine(d);

    d->process.write("isready\n");
    d->process.waitForReadyRead();
    QByteArray ba = d->process.readLine();
    while(0 < ba.length() && !ba.contains("readyok")){
        ba = d->process.readLine();
    }
    if(!ba.contains("readyok")){
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Engine unable to get ready");
    }

    // Start the background worker
    connect(&d->process, SIGNAL(readyRead()), this, SLOT(_data_available()));
    d->writer_thread = QtConcurrent::run(this, &UCI_Client::_writer_thread);
}

UCI_Client::~UCI_Client()
{
    G_D;
    d->lock.lock();
    d->cancel_thread = true;
    d->wc.wakeOne();
    d->lock.unlock();

    d->writer_thread.waitForFinished();
    d->process.close();
    d->process.waitForFinished();
    G_D_UNINIT();
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

void UCI_Client::SetDebugOutputEnabled(bool e)
{
    G_D;
    d->debug_output_enabled = e;
    _append_to_write_queue(QString("debug %1").arg(e ? "on" : "off").toUtf8());
}

bool UCI_Client::GetDebugOutputEnabled() const
{
    G_D;
    return d->debug_output_enabled;
}

void UCI_Client::SetAnalysisMode(bool b)
{
    G_D;
    d->analysis_mode = b;
    _append_to_write_queue(QString("setoption name "UCI_ANALYSE_MODE_TAG" value %1").arg(b ? "true" : "false").toUtf8());
}

bool UCI_Client::GetAnalysisMode() const
{
    G_D;
    return d->analysis_mode;
}

void UCI_Client::_writer_thread()
{
    G_D;
    d->lock.lock();
    while(!d->cancel_thread)
    {
        while(!d->cancel_thread && d->outqueue.length() == 0){
            d->wc.wait(&d->lock);
        }

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

void UCI_Client::_data_available()
{
    G_D;
    QByteArray ba = d->process.readLine();
    while(0 < ba.length())
    {
        // remove the new line at the end (can be CR or LF or any combination)
        if(ba[ba.length() - 1] == (char)0x0a || ba[ba.length() - 1] == (char)0x0d)
            ba.chop(1);
        if(ba[ba.length() - 1] == (char)0x0a || ba[ba.length() - 1] == (char)0x0d)
            ba.chop(1);

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

void UCI_Client::SetPosition(const QByteArray &data)
{
    G_D;
    d->lock.lock();
    d->outqueue.append(QString("position fen %1").arg(data.constData()).toUtf8());
    d->lock.unlock();
    d->wc.wakeOne();
}

void UCI_Client::Go(const UCI_Client::GoParams &params)
{
    QString str = "go ";
    if(-1 == params.MoveTime)
        str.append("infinite");
    else
        str.append(QString("movetime %1").arg(params.MoveTime));
    _append_to_write_queue(str.toUtf8());
}

void UCI_Client::Stop()
{
    _append_to_write_queue("stop");
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
