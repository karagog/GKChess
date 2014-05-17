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
#include "gkchess_uiglobals.h"
#include "mainwindow.h"
#include "gutil_messageboxlogger.h"
#include "gutil_about.h"
#include "gutil_persistentdata.h"
USING_NAMESPACE_GUTIL1(QT);


#define APPLICATION_ICON ":/gkchess/icons/default/k.png"


#define SETTINGS_MAINWINDOW_STATE "mainwindow_state"
#define SETTINGS_MAINWINDOW_GEOMETRY "mainwindow_geometry"


PositionExplorerApplication::PositionExplorerApplication(int &argc, char **argv)
    :Application(argc, argv, "Position Explorer"),
      m_mainWindow(new MainWindow),
      m_settings(new PersistentData("settings", "main", this))
{
    setApplicationName("GKChess Studio");
    setApplicationVersion("0.0.0");

    // Make sure the GKChess resources are initialized
    GKChess::UI::InitializeApplicationResources();
    setWindowIcon(QIcon(APPLICATION_ICON));

    if(m_settings->Contains(SETTINGS_MAINWINDOW_STATE))
        m_mainWindow->restoreState(m_settings->Value(SETTINGS_MAINWINDOW_STATE).toByteArray());
    if(m_settings->Contains(SETTINGS_MAINWINDOW_GEOMETRY))
        m_mainWindow->restoreGeometry(m_settings->Value(SETTINGS_MAINWINDOW_GEOMETRY).toByteArray());

    m_mainWindow->show();
}


void PositionExplorerApplication::about_to_quit()
{
    QByteArray mainWindow_state = m_mainWindow->saveState();
    QByteArray mainWindow_geometry = m_mainWindow->saveGeometry();
    m_mainWindow->hide();
    m_mainWindow->deleteLater();

    m_settings->SetValue(SETTINGS_MAINWINDOW_STATE, mainWindow_state);
    m_settings->SetValue(SETTINGS_MAINWINDOW_GEOMETRY, mainWindow_geometry);
}

void PositionExplorerApplication::handle_exception(const GUtil::Exception<> &ex)
{
    MessageBoxLogger().LogException(ex);
}


class __about : public GUtil::QT::About
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


void PositionExplorerApplication::show_about(QWidget *w)
{
    __about(w).ShowAbout();
}

