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

#ifndef GKCHESS_ENGINEMANAGER_H
#define GKCHESS_ENGINEMANAGER_H

#include <QString>
#include <QPluginLoader>

namespace GKChess{


class IEngine;
class EngineSettings;


class EngineManager
{
    void *d;
public:

    /** Initializes the engine, or throws an exception if something goes wrong. */
    EngineManager(const QString &engine_name, EngineSettings *);
    ~EngineManager();

    /** Returns the name of the engine that was passed to the constructor. */
    QString GetEngineName() const;

    /** Returns a reference to the engine. */
    IEngine &GetEngine() const;

    /** Causes the current settings to be applied to the engine. */
    void ApplySettings();

};


}

#endif // GKCHESS_ENGINEMANAGER_H
