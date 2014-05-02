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
#include <QProcess>
#include <QFile>
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;


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

static void __start_and_validate_engine(QProcess &p, const QString &e, UCI_Client::EngineInfo *info = 0)
{
    if(!QFile(e).exists())
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "File does not exist");

    p.setReadChannel(QProcess::StandardOutput);
    //p.setProcessChannelMode(QProcess::MergedChannels);
    p.start(e);
    if(!p.waitForStarted())
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Engine unable to start");

    // Ignore the first bit of junk the engine sends
    p.waitForReadyRead();
    p.readAllStandardOutput();

    GASSERT(p.state() == QProcess::Running);

    bool valid = false;
    QString name;

    p.write("uci\n");
    p.waitForBytesWritten();
    p.waitForReadyRead();
    QByteArray output = p.readAllStandardOutput();

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
                if(info)
                    info->Name = name;
            }
            if(-1 != author_idx && info){
                info->Author = line.right(line.length() - (author_idx + 7));
            }
        }
        else if(line.startsWith("option"))
        {
            if(info)
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
                    opt = new UCI_Client::CheckOption(opt_name, val_str.toLower() == "true");
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
                    info->Options.Insert(opt_name, opt);
            }
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
    :QThread(parent),
      m_engine(path_to_engine)
{
    // Make sure the engine exists and is valid, and populate the available options
    QProcess p;
    try{
        __start_and_validate_engine(p, m_engine, &m_info);
    }catch(...){
        p.close();
        throw;
    }
    p.close();
}

UCI_Client::EngineInfo::~EngineInfo()
{
    for(typename Map<QString, Option_t *>::iterator iter = Options.begin();
        iter != Options.end();
        ++iter)
        delete iter->Value();
}

void UCI_Client::Startup()
{
    start();
}

void UCI_Client::run()
{
    QProcess p;
    __start_and_validate_engine(p, m_engine);

    G_FOREVER
    {

    }
    p.close();
}


END_NAMESPACE_GKCHESS;
