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
#include "gkchess_pgn_player.h"
#include "gkchess_movehistorycontrol.h"
#include "gkchess_enginecontrol.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

namespace GUtil{ namespace QT{
class PersistentData;
}}

namespace GKChess{ namespace UI{
class PGN_PlayerControl;
class BookReader;
}}


class MainWindow : public QMainWindow
{
    Q_OBJECT

    Ui::MainWindow *ui;
    GKChess::ObservableBoard m_board;
    GKChess::UI::PGN_PlayerControl *m_pgnPlayer;
    GKChess::UI::MoveHistoryControl *m_moveHistory;
    GKChess::UI::EngineControl *m_engineControl;
    GKChess::UI::ColoredPieceIconFactory m_iconFactory;

    GUtil::QT::PersistentData *m_settings;
    GUtil::QT::PersistentData *m_engineSettings;

public:

    explicit MainWindow(GUtil::QT::PersistentData *settings,
                        GUtil::QT::PersistentData *engine_settings,
                        QWidget *parent = 0);
    ~MainWindow();


protected:

    void closeEvent(QCloseEvent *);


private slots:

    void _load_fen_clipboard();
    void _load_pgn_clipboard();
    void _load_pgn_file();
    void _position_to_clipboard();

    void _opening_book_reader();
    void _show_move_history();
    void _show_engine_control();

    void _setup_standard_chess();
    void _setup_random_chess960();

    void _random_chess960_position();

#ifdef DEBUG
    void _piece_moved(const GKChess::MoveData &);
#endif


private:

    void _load_fen_string(const GUtil::String &);
    void _load_pgn_string(const GUtil::String &);

    bool eventFilter(QObject *, QEvent *);

};

#endif // MAINWINDOW_H
