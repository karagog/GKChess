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
#include "gkchess_bookreadercontrol.h"
#include "gkchess_manage_engines.h"
#include "gutil_file.h"
#include "gutil_application.h"
#include "gutil_persistentdata.h"
#include <QClipboard>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QWhatsThis>
#include <QCloseEvent>
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(QT);
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GKCHESS1(UI);

#define SETTINGS_MAINWINDOW_STATE "mainwindow_state"
#define SETTINGS_MAINWINDOW_GEOMETRY "mainwindow_geometry"

#define SETTINGS_DW_PGNPLAYER_OPEN  "mainwindow_pgnplayer_open"
#define SETTINGS_DW_ENGINECONTROL_OPEN  "mainwindow_enginecontrol_open"
#define SETTINGS_DW_MOVEHISTORY_OPEN  "mainwindow_movehistory_open"


MainWindow::MainWindow(PersistentData *settings,
                       EngineSettings *engine_settings,
                       QWidget *parent)
    :QMainWindow(parent),
      m_board(),
      //m_board(10),
      m_iconFactory(":/gkchess/icons/default", Qt::white, Qt::gray),
      ui(new Ui::MainWindow),
      m_settings(settings),
      m_engineSettings(engine_settings)
{
    ui->setupUi(this);

    ui->menuHelp->insertAction(ui->actionAbout, QWhatsThis::createAction(this));

    ui->dw_moveHistory->setWidget(new MoveHistoryControl(m_board, ui->dw_moveHistory));
    ui->dw_pgnPlayer->setWidget(new UI::PGN_PlayerControl(m_board, ui->dw_pgnPlayer));
    ui->dw_engineControl->setWidget(new EngineControl(m_board, m_engineSettings, m_settings, ui->dw_engineControl));
    ui->dw_bookReader->setWidget(new UI::BookReaderControl(m_board, m_settings, ui->dw_bookReader));

    // Catch events on the boardview so we can customize certain behaviors (like
    //  the context menu)
    ui->boardView->installEventFilter(this);

    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionLoad_FEN_in_Clipboard, SIGNAL(triggered()), this, SLOT(_load_fen_clipboard()));
    connect(ui->actionLoad_PGN_in_Clipboard, SIGNAL(triggered()), this, SLOT(_load_pgn_clipboard()));
    connect(ui->actionLoadPGN_File, SIGNAL(triggered()), this, SLOT(_load_pgn_file()));
    connect(ui->actionPosition_to_Clipboard, SIGNAL(triggered()), this, SLOT(_position_to_clipboard()));
    connect(ui->actionRandom_Chess960_Position, SIGNAL(triggered()), this, SLOT(_random_chess960_position()));
    connect(ui->actionPGN_Player, SIGNAL(triggered()), ui->dw_pgnPlayer, SLOT(show()));
    connect(ui->actionOpening_Book_Reader, SIGNAL(triggered()), ui->dw_bookReader, SLOT(show()));
    connect(ui->actionMove_History, SIGNAL(triggered()), ui->dw_moveHistory, SLOT(show()));
    connect(ui->action_Engine_Control, SIGNAL(triggered()), ui->dw_engineControl, SLOT(show()));
    connect(ui->actionStandard_Starting_Position, SIGNAL(triggered()), this, SLOT(_setup_standard_chess()));
    connect(ui->actionChess960_Starting_Position, SIGNAL(triggered()), this, SLOT(_setup_random_chess960()));
    connect(ui->actionManage_Engines, SIGNAL(triggered()), this, SLOT(_manage_engines()));
    connect(ui->action_FlipBoard, SIGNAL(triggered()), ui->boardView, SLOT(FlipOrientation()));

    connect(ui->actionAbout, SIGNAL(triggered()), gApp, SLOT(About()));

    _setup_standard_chess();

    // For testing 10-column boards:
    //m_board.FromFEN("rnbqkbnrnn/pppppppppp/10/10/10/10/PPPPPPPPPP/RNBQKBNRNN w KQkq - 0 1");

#ifdef DEBUG
    connect(&m_board, SIGNAL(NotifyPieceMoved(const GKChess::MoveData &)),
            this, SLOT(_piece_moved(const GKChess::MoveData &)));
#endif

    ui->boardView->SetIconFactory(&m_iconFactory);
    ui->boardView->SetBoard(&m_board);
    ui->boardView->SetShowThreatCounts(true);

    // Restore the last state of the main window
    if(m_settings->Contains(SETTINGS_MAINWINDOW_STATE))
        restoreState(m_settings->Value(SETTINGS_MAINWINDOW_STATE).toByteArray());
    if(m_settings->Contains(SETTINGS_MAINWINDOW_GEOMETRY))
        restoreGeometry(m_settings->Value(SETTINGS_MAINWINDOW_GEOMETRY).toByteArray());
}

MainWindow::~MainWindow()
{
    delete ui;
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

            ui->menuBoard->move(cmev->globalPos());
            ui->menuBoard->show();

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
    ui->dw_pgnPlayer->show();

    static_cast<PGN_PlayerControl *>(ui->dw_pgnPlayer->widget())->LoadPGN(s);
}

void MainWindow::_random_chess960_position()
{
    UI::Chess960Generator *dlg = new UI::Chess960Generator(this);
    dlg->show();
}

void MainWindow::_manage_engines()
{
    ManageEngines mng(m_engineSettings, m_settings, this);
    mng.exec();
}

void MainWindow::_setup_standard_chess()
{
    m_board.SetupNewGame(Board::SetupStandardChess);
}

void MainWindow::_setup_random_chess960()
{
    m_board.SetupNewGame(Board::SetupChess960);
}

void MainWindow::_position_to_clipboard()
{
    if(QApplication::clipboard())
        QApplication::clipboard()->setText(m_board.ToFEN().ToQString());
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    m_settings->SetValue(SETTINGS_MAINWINDOW_STATE, saveState());
    m_settings->SetValue(SETTINGS_MAINWINDOW_GEOMETRY, saveGeometry());

    QMainWindow::closeEvent(ev);
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
