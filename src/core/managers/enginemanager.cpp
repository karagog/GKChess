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
#include <gkchess_common.h>
#include "gkchess_enginesettings.h"
#include <gutil/pluginutils.h>
#include <gutil/exception.h>
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(Qt);

namespace{
struct d_t
{
    QString engine_name;
    QPluginLoader pluginloader;
    GKChess::IEngine *engine;
    GKChess::EngineSettings *engine_settings;
};
}

NAMESPACE_GKCHESS;


EngineManager::EngineManager(const QString &engine_name, EngineSettings *settings)
{
    G_D_INIT();
    G_D;

    if(!settings->GetEngineList().contains(engine_name))
        throw Exception<>("Unrecognized Engine");

    d->engine_name = engine_name;
    d->engine_settings = settings;

    // Load the plugin and start the engine
    d->engine = PluginUtils::LoadPlugin<IEngine>(d->pluginloader, "uciEnginePlugin")->Create();
    d->engine->StartEngine(settings->GetEnginePath(engine_name));

    ApplySettings();
}

EngineManager::~EngineManager()
{
    G_D;
    d->engine->StopEngine();
    d->engine->deleteLater();
    G_D_UNINIT();
}

void EngineManager::ApplySettings()
{
    G_D;

    // Go through the settings and apply our configuration
    QVariantMap options = d->engine_settings->GetOptionsForEngine(d->engine_name);
    foreach(const QString &k, options.keys()){
        d->engine->SetOption(k, options[k]);
    }
}

IEngine &EngineManager::GetEngine() const
{
    G_D;
    return *d->engine;
}

QString EngineManager::GetEngineName() const
{
    G_D;
    return d->engine_name;
}


END_NAMESPACE_GKCHESS;
