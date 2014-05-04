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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gkchess_pgn_parser.h"
#include <QClipboard>
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GKCHESS;

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_board(),
      //m_board(10),
      m_uci(new UCI_Client("/usr/games/stockfish")),
      m_iconFactory(":/gkchess/icons/default", Qt::white, Qt::gray)
{
    ui->setupUi(this);

    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionLoad_FEN_in_Clipboard, SIGNAL(triggered()), this, SLOT(_load_fen_clipboard()));

    m_board.SetupNewGame(Board::SetupStandardChess);
    //m_board.SetupNewGame(Board::SetupChess960);

    // For testing 10-column boards:
    //m_board.FromFEN("rnbqkbnrnn/pppppppppp/10/10/10/10/PPPPPPPPPP/RNBQKBNRNN w KQkq - 0 1");

    ui->engine_control->Initialize(m_uci, &m_board);

#ifdef DEBUG
    connect(&m_board, SIGNAL(NotifyPieceMoved(const GKChess::MoveData &)),
            this, SLOT(_piece_moved(const GKChess::MoveData &)));
#endif

    ui->boardView->SetIconFactory(&m_iconFactory);
    ui->boardView->SetBoard(&m_board);
    ui->boardView->SetShowThreatCounts(true);

    m_iconFactory.ChangeColors(Qt::yellow, Qt::gray);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_uci;
}

static QString __get_clipboard_data()
{
    return QApplication::clipboard() ? QApplication::clipboard()->text() : QString();
}

void MainWindow::_load_fen_clipboard()
{
    return _load_fen_string(String::FromQString(__get_clipboard_data().trimmed()));
}

void MainWindow::_load_fen_string(const String &s)
{
    Board new_board;
    new_board.FromFEN(s);

    // We assign only after making sure that the fen string is correct
    m_board = new_board;
}





#ifdef DEBUG
void MainWindow::_piece_moved(const GKChess::MoveData &)
{
    GUtil::String fen_string1 = m_board.ToFEN();
    Board cpy;
    cpy.FromFEN(fen_string1);

    GUtil::String fen_string2 = cpy.ToFEN();
    GASSERT2(fen_string1 == fen_string2,
             GUtil::String::Format("%s != %s",
                                   fen_string1.ConstData(),
                                   fen_string2.ConstData()).ConstData());
    GDEBUG(fen_string1);

    //m_board.ShowIndex();
}
#endif
