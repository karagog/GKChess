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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gkchess_board.h"
#include "gkchess_coloredpieceiconfactory.h"
#include "gkchess_uci_client.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Ui::MainWindow *ui;
    GKChess::ObservableBoard m_board;
    GKChess::UCI_Client *m_uci;

    GKChess::UI::ColoredPieceIconFactory m_iconFactory;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private slots:

    void _load_fen_clipboard();

#ifdef DEBUG
    void _piece_moved(const GKChess::MoveData &);
#endif


private:

    void _load_fen_string(const GUtil::String &);

};

#endif // MAINWINDOW_H
