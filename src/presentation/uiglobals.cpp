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

/** \file This file takes care of initializing any globals used by the library. */

#include "uiglobals.h"
#include <QResource>
#include <QFontDatabase>
#include <QDirIterator>

static bool __was_initialized(false);

// This has to be outside of a namespace, otherwise Q_INIT_RESOURCES doesn't work
static void __init_app_resources()
{
    if(!__was_initialized)
    {
        // Initialize the resources for icons and fonts
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(icons_default);

        // Add the fonts to the application's font database
        QDirIterator iter(":/gkchess/fonts");
        while(iter.hasNext())
        {
            iter.next();
            int handle = QFontDatabase::addApplicationFont(iter.filePath());

            QStringList families = QFontDatabase::applicationFontFamilies(handle);
            foreach(QString f, families)
                qDebug("Found font: '%s'", f.toUtf8().constData());

            //qDebug("Found font: '%s'", iter.fileName().toUtf8().constData());
        }

        __was_initialized = true;
    }
}


NAMESPACE_GKCHESS1(UI);


void InitializeApplicationResources()
{
    __init_app_resources();
}


END_NAMESPACE_GKCHESS1;
