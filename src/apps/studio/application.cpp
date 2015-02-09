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

#include "application.h"
#include "mainwindow.h"
#include "gkchess_uiglobals.h"
#include "gkchess_enginesettings.h"
#include <gutil/messageboxlogger.h>
#include <gutil/about.h>
#include <gutil/qt_settings.h>
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GUTIL1(Qt);


#define APPLICATION_ICON ":/gkchess/icons/default/k.png"

#define APPLICATION_NAME "GKChess Studio"
#define APPLICATION_VERSION "0.0.0"


StudioApplication::StudioApplication(int &argc, char **argv)
    :Application(argc, argv, APPLICATION_NAME, APPLICATION_VERSION)
{
    // Initialize these settings before setting the application name, so they are shared between apps
    m_engineSettings = new EngineSettings;

    // Initialize these settings after setting the application name, so that they are specific to this app
    m_settings = new GUtil::Qt::Settings("main", "", this);
    m_mainWindow = new MainWindow(m_settings, m_engineSettings);

    // Make sure the GKChess resources are initialized
    GKChess::UI::InitializeApplicationResources();
    setWindowIcon(QIcon(APPLICATION_ICON));

    m_mainWindow->setWindowTitle(APPLICATION_NAME);
    m_mainWindow->show();
}


void StudioApplication::about_to_quit()
{
    m_mainWindow->hide();
    m_mainWindow->deleteLater();

    delete m_engineSettings;
}

void StudioApplication::handle_exception(const GUtil::Exception<> &ex)
{
    MessageBoxLogger().LogException(ex);
}


class __about : public GUtil::Qt::About
{
public:
    __about(QWidget *w)
        :About(w, true, true)
    {
        SetImage(APPLICATION_ICON);
        SetWindowTitle(QString("About %1").arg("GKChess Studio"));
        _header.setText("GKChess Studio");

        _text.setText("This application was developed by George Karagoulis.\n\n"

                      "It was intended to be free for everyone to use."
                      " I hope you enjoy it, and don't feel guilty about benefiting from my"
                      " slave labor; I wrote this mostly for myself anyways."
                      );

        _buildinfo.setText(QString("Version %1").arg(gApp->applicationVersion()));
    }

};


void StudioApplication::show_about(QWidget *w)
{
    (new __about(w))->ShowAbout();
}

