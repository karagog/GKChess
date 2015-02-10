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

#ifndef GKCHESS_ENGINECONTROL_H
#define GKCHESS_ENGINECONTROL_H

#include "gkchess_enginemanager.h"
#include <gutil/smartpointer.h>
#include <gutil/qt_settings.h>
#include <QWidget>

namespace Ui {
class EngineControl;
}

namespace GKChess{
class IEngine;
class Board;
class EngineSettings;
class GenericMove;

namespace UI{


/** Provides buttons and other widgets to control the function of the engine */
class EngineControl : public QWidget
{
    Q_OBJECT

    GUtil::SmartPointer<EngineManager> m_engineMan;
    Board &m_board;
    EngineSettings *m_settings;
    GUtil::Qt::Settings *m_appSettings;
    QStringList m_engineList;
    bool m_suppressUpdate;

    Ui::EngineControl *ui;
public:

    explicit EngineControl(Board &, EngineSettings *engine_settings, GUtil::Qt::Settings *app_settings, QWidget *parent = 0);
    ~EngineControl();


public slots:

    /** Tells the engine to start thinking. */
    void Go();

    /** Tells the engine to stop thinking and give a best move suggestion. */
    void Stop();


private slots:

    void _msg_tx(const QByteArray &);
    void _msg_rx(const QByteArray &);
    void _best_move_received(const GenericMove &, const GenericMove &);

    void _engine_crashed();

    void _go_stop_pressed();

    void _engine_selection_changed(const QString &);
    void _engine_selection_activated(int);

    void _engines_updated();


private:

    void _update_go_stop_text(bool);

};


}}

#endif // GKCHESS_ENGINECONTROL_H
