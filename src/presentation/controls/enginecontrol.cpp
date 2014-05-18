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

#include "enginecontrol.h"
#include "manage_engines.h"
#include "ui_enginecontrol.h"
#include "gkchess_board.h"
#include "gkchess_uci_client.h"
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GUTIL1(QT);
USING_NAMESPACE_GUTIL;

#ifdef Q_OS_UNIX
    #define STOCKFISH_PATH  "/usr/games/stockfish"
#else
    #define STOCKFISH_PATH  "stockfish.exe"
#endif

NAMESPACE_GKCHESS1(UI);


EngineControl::EngineControl(Board *b, PersistentData *pd, QWidget *parent)
    :QWidget(parent),
      m_board(b),
      m_settings(pd),
      ui(new Ui::EngineControl)
{
    ui->setupUi(this);

    _refresh_combo_box();
}

EngineControl::~EngineControl()
{
    delete ui;
}

void EngineControl::_refresh_combo_box()
{
    ui->cmb_engine->clear();

    m_settingsKeys = m_settings->Keys();
    m_settingsKeys.Sort();
    if(0 < m_settingsKeys.Length()){
        int i = 0;
        G_FOREACH_CONST(const String &k, m_settingsKeys){
            ui->cmb_engine->addItem(k, i);
            ++i;
        }
        ui->wdg_control->setEnabled(true);
    }
    else{
        ui->cmb_engine->addItem(tr("(none)"), -1);
        ui->wdg_control->setEnabled(false);
    }
}

void EngineControl::Go()
{
    m_engineMan->GetEngine().SetPosition(m_board->ToFEN());

    IEngine::ThinkParams p;
    p.MoveTime = ui->spin_thinkTime->value();
    p.Nodes = ui->spin_nodes->value();
    p.Depth = ui->spin_depth->value();
    p.Mate = ui->spin_mate->value();
    m_engineMan->GetEngine().StartThinking(p);
}

void EngineControl::Stop()
{
    m_engineMan->GetEngine().StopThinking();
}

void EngineControl::_msg_rx(const QByteArray &line)
{
    if(ui->tb_engineLog->isVisible())
        ui->tb_engineLog->append(line);
}

void EngineControl::_best_move_received(const QByteArray &, const QByteArray &)
{
    ui->btn_gostop->setChecked(false);
}

void EngineControl::_engine_crashed()
{
    ui->tb_engineLog->append(tr("*** ENGINE CRASHED ***"));
}

void EngineControl::_go_stop_pressed()
{
    if(ui->btn_gostop->isChecked()){
        Go();
        ui->btn_gostop->setText(tr("Stop"));
    }
    else{
        Stop();
        ui->btn_gostop->setText(tr("Go"));
    }
}

void EngineControl::_engine_selection_changed()
{
    const int indx = ui->cmb_engine->currentIndex();
    if(0 == indx && ui->cmb_engine->currentText() == "(none)"){
        // Open the engine settings manager
        ManageEngines mng(m_settings, this);
        mng.exec();
    }
    else{
        m_engineMan = new EngineManager(STOCKFISH_PATH);
        connect(&m_engineMan->GetEngine(), SIGNAL(MessageReceived(QByteArray)), this, SLOT(_msg_rx(QByteArray)));
        connect(&m_engineMan->GetEngine(), SIGNAL(NotifyEngineCrashed()), this, SLOT(_engine_crashed()));
        connect(&m_engineMan->GetEngine(), SIGNAL(BestMove(QByteArray,QByteArray)),
                this, SLOT(Stop()));
        ui->wdg_control->setEnabled(true);
    }
}


END_NAMESPACE_GKCHESS1;
