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

#ifndef GKCHESS_MOVEHISTORY_CONTROL_H
#define GKCHESS_MOVEHISTORY_CONTROL_H

#include "gkchess_moverecorderplayer.h"
#include <QWidget>

namespace Ui {
class MoveHistoryControl;
}

namespace GKChess{
class ObservableBoard;
class Board;

namespace UI{


class MoveHistoryControl :
        public QWidget
{
    Q_OBJECT
    Board const &m_board;
    GKChess::MoveRecorderPlayer m_recorder;
    Ui::MoveHistoryControl *ui;
public:
    explicit MoveHistoryControl(ObservableBoard &, QWidget *parent = 0);
    ~MoveHistoryControl();

private slots:

    void goto_first();
    void go_forward();
    void go_back();

    void history_updated();

};


}}


#endif // GKCHESS_MOVEHISTORY_CONTROL_H
