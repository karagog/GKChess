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

#ifndef GKCHESS_ENGINESETTINGS_H
#define GKCHESS_ENGINESETTINGS_H

#include "gutil_persistentdata.h"
#include <QStringList>

namespace GKChess
{


/** A class to encapsulate global storage of our engine settings. */
class EngineSettings :
        public QObject
{
    Q_OBJECT
    GUtil::QT::PersistentData m_data;
public:

    /** You should construct this before setting the application name, so the data gets shared between all
     *  applications.
    */
    EngineSettings(QObject * = 0);

    /** Returns the list of configured engines. */
    QStringList GetEngineList() const;

    /** Sets the option for the engine.  If the engine is not in the database yet, it will
     *  be added with the given options.
    */
    void SetOptionsForEngine(const QString &engine, const QVariantMap &);

    /** Returns the options for the given engine.  Or an empty map if the engine was not configured. */
    QVariantMap GetOptionsForEngine(const QString &engine) const;


signals:

    /** This is emitted whenever the list of engines were updated.
     *  This could happen because you updated it yourself, or another application updated it.
    */
    void NotifyEnginesUpdated();

};


}

#endif // GKCHESS_ENGINESETTINGS_H
