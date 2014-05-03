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

#include <QWidget>

namespace Ui {
class EngineControl;
}

namespace GKChess{
class UCI_Client;
class Board;

namespace UI{


/** Provides buttons and other widgets to control the function of the engine */
class EngineControl : public QWidget
{
    Q_OBJECT

    UCI_Client *m_uci;
    Board *m_board;

    Ui::EngineControl *ui;
public:
    explicit EngineControl(QWidget *parent = 0);
    ~EngineControl();

    /** You must call this before the control will work. */
    void Initialize(UCI_Client *, Board *);
    void Uninitialize();
    bool IsInitialized() const{ return m_uci; }


public slots:

    /** Tells the engine to start thinking. */
    void Go();

    /** Tells the engine to stop thinking and give a best move suggestion. */
    void Stop();


private slots:

    void _msg_rx(const QByteArray &);
    void _engine_crashed();

};


}}

#endif // GKCHESS_ENGINECONTROL_H
