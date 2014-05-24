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

#include "enginemanager.h"
#include "gkchess_iengine.h"
#include "gkchess_globals.h"
#include "gutil_pluginutils.h"
USING_NAMESPACE_GUTIL;

namespace{
struct d_t
{
    QPluginLoader pluginloader;
    GKChess::IEngine *engine;
};
}

NAMESPACE_GKCHESS;


EngineManager::EngineManager(const QString &path)
{
    G_D_INIT();
    G_D;

    // Load the plugin and start the engine
    d->engine = PluginUtils::LoadPlugin<IEngine>(d->pluginloader, "uciEnginePlugin")->Create();
    d->engine->StartEngine(path);
}

EngineManager::~EngineManager()
{
    G_D;
    d->engine->StopEngine();
    d->engine->deleteLater();
    G_D_UNINIT();
}

IEngine &EngineManager::GetEngine() const
{
    G_D;
    return *d->engine;
}


END_NAMESPACE_GKCHESS;
