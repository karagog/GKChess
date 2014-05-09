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
#include "gkchess_pgn_playercontrol.h"
#include "gkchess_chess960generatorcontrol.h"
#include "gutil_file.h"
#include "gutil_application.h"
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDockWidget>
#include <QFileDialog>
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GKCHESS1(UI);

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_board(),
      //m_board(10),
      m_pgnPlayer(new UI::PGN_PlayerControl(&m_board, this)),
      m_uci(new UCI_Client("/usr/games/stockfish")),
      m_iconFactory(":/gkchess/icons/default", Qt::yellow, Qt::gray)
{
    ui->setupUi(this);

    // Add the pgn control dock widget
    QDockWidget *dock_widget = new QDockWidget(tr("PGN Control"), this);
    dock_widget->setWidget(m_pgnPlayer);
    addDockWidget(Qt::LeftDockWidgetArea, dock_widget);

    ui->boardView->installEventFilter(this);

    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionLoad_FEN_in_Clipboard, SIGNAL(triggered()), this, SLOT(_load_fen_clipboard()));
    connect(ui->actionLoad_PGN_in_Clipboard, SIGNAL(triggered()), this, SLOT(_load_pgn_clipboard()));
    connect(ui->actionLoadPGN_File, SIGNAL(triggered()), this, SLOT(_load_pgn_file()));
    connect(ui->actionRandom_Chess960_Position, SIGNAL(triggered()), this, SLOT(_random_chess960_position()));

    connect(ui->actionAbout, SIGNAL(triggered()), gApp, SLOT(About()));

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
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_uci;
}

bool MainWindow::eventFilter(QObject *o, QEvent *ev)
{
    bool ret = false;
    if(o == ui->boardView)
    {
        switch(ev->type())
        {
        case QEvent::ContextMenu:
        {
            // Display the context menu:
            QContextMenuEvent *cmev = static_cast<QContextMenuEvent *>(ev);

            QMenu *menu = new QMenu(this);
            menu->addMenu(ui->menu_Load_Position);
            menu->move(cmev->globalPos());
            menu->show();

            ev->accept();
            ret = true;
        }
            break;
        default: break;
        }
    }
    return ret;
}

static QString __get_clipboard_data()
{
    return QApplication::clipboard() ? QApplication::clipboard()->text() : QString();
}

void MainWindow::_load_fen_clipboard()
{
    _load_fen_string(String::FromQString(__get_clipboard_data().trimmed()));
}

void MainWindow::_load_pgn_clipboard()
{
    _load_pgn_string(String::FromQString(__get_clipboard_data().trimmed()));
}

void MainWindow::_load_pgn_file()
{
    QString fn = QFileDialog::getOpenFileName(this, "Select PGN", QString(), "*.pgn");
    QFile f(fn);
    if(!f.open(QFile::ReadOnly))
        THROW_NEW_GUTIL_EXCEPTION2(Exception, String::Format("Could not open file: %s", fn.toUtf8().constData()));
    QString s = f.readAll();
    f.close();
    _load_pgn_string(String::FromQString(s));
}

void MainWindow::_load_fen_string(const String &s)
{
    Board new_board;
    new_board.FromFEN(s);

    // We assign only after making sure that the fen string is correct
    m_board = new_board;
}

void MainWindow::_load_pgn_string(const String &s)
{
    m_pgnPlayer->LoadPGN(s);
}

void MainWindow::_random_chess960_position()
{
    UI::Chess960Generator *dlg = new UI::Chess960Generator(this);
    dlg->show();
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
    GDEBUG(fen_string2);

    //m_board.ShowIndex();
}
#endif
