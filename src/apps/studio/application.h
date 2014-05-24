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

#ifndef APPLICATION_H
#define APPLICATION_H

#include "gutil_application.h"

namespace GUtil{ namespace QT{
class PersistentData;
}}

namespace GKChess{
class EngineSettings;
}

class MainWindow;

class StudioApplication :
        public GUtil::QT::Application
{
    Q_OBJECT
    GKChess::EngineSettings *m_engineSettings;
    GUtil::QT::PersistentData *m_settings;
    MainWindow *m_mainWindow;

public:
    explicit StudioApplication(int &argc, char **argv);


protected:

    virtual void handle_exception(const GUtil::Exception<> &);

private:
    virtual void about_to_quit();
    virtual void show_about(QWidget *);
};

#endif // APPLICATION_H
