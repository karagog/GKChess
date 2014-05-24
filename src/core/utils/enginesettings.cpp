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


void EngineSettings::SetOptionsForEngine(const QString &engine, const QVariantMap &vals)
{
    m_data.SetValue(engine, vals);
}

QVariantMap EngineSettings::GetOptionsForEngine(const QString &engine) const
{
    QVariantMap ret;
    if(m_data.Contains(engine))
        ret = m_data.Value(engine).toMap();
    return ret;
}


END_NAMESPACE_GKCHESS;
