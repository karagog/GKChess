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
USING_NAMESPACE_GUTIL1(QT);


PositionExplorerApplication::PositionExplorerApplication(int &argc, char **argv)
    :Application(argc, argv, "Position Explorer"),
      m_mainWindow(new MainWindow)
{
    setApplicationName("GKChess Position Explorer");
    setApplicationVersion("0.0.0");

    // Make sure the GKChess resources are initialized
    GKChess::UI::InitializeApplicationResources();
    setWindowIcon(QIcon(":/gkchess/icons/default/k.png"));

    m_mainWindow->show();
}


void PositionExplorerApplication::about_to_quit()
{
    m_mainWindow->hide();
    m_mainWindow->deleteLater();
}

void PositionExplorerApplication::handle_exception(const GUtil::Exception<> &ex)
{
    MessageBoxLogger().LogException(ex);
}
