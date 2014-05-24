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

#include "enginesettings.h"
#include "gkchess_globals.h"
USING_NAMESPACE_GUTIL;

// This needs to never collide with any potential engine settings names, because it will be
//  stored in a map with option names as keys.
#define PATH_SETTING_STRING "gkchess_engine_path"

NAMESPACE_GKCHESS;


EngineSettings::EngineSettings(QObject *p)
    :QObject(p),
      m_data("GKChess_GlobalEngineSettings")
{
    connect(&m_data, SIGNAL(DataChanged()), this, SIGNAL(NotifyEnginesUpdated()));
}

QStringList EngineSettings::GetEngineList() const
{
    QStringList ret = m_data.Keys();
    ret.sort();
    return ret;
}

void EngineSettings::RemoveEngine(const QString &name)
{
    m_data.RemoveValue(name);
}

QString EngineSettings::GetEnginePath(const QString &name)
{
    QString path;
    if(m_data.Contains(name)){
        path = m_data.Value(name).toMap().value(PATH_SETTING_STRING).toString();
    }
    return path;
}

void EngineSettings::SetEnginePath(const QString &name, const QString &path)
{
    QVariantMap vals = m_data.Value(name).toMap();
    vals[PATH_SETTING_STRING] = path;
    m_data.SetValue(name, vals);
}

void EngineSettings::SetOptionsForEngine(const QString &engine, const QVariantMap &vals)
{
    m_data.SetValue(engine, vals);
}

QVariantMap EngineSettings::GetOptionsForEngine(const QString &engine) const
{
    QVariantMap ret;
    if(m_data.Contains(engine)){
        ret = m_data.Value(engine).toMap();
        ret.remove(PATH_SETTING_STRING);
    }
    return ret;
}


END_NAMESPACE_GKCHESS;
