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
#include "ui_enginecontrol.h"
#include "gkchess_board.h"
#include "gkchess_uci_client.h"
USING_NAMESPACE_GKCHESS;

#ifdef Q_OS_UNIX
    #define STOCKFISH_PATH  "/usr/games/stockfish"
#else
    #define STOCKFISH_PATH  "stockfish.exe"
#endif

NAMESPACE_GKCHESS1(UI);


EngineControl::EngineControl(Board *b, QWidget *parent)
    :QWidget(parent),
      m_engineMan(STOCKFISH_PATH),
      m_board(b),
      ui(new Ui::EngineControl)
{
    ui->setupUi(this);

    connect(&m_engineMan.GetEngine(), SIGNAL(MessageReceived(QByteArray)), this, SLOT(_msg_rx(QByteArray)));
    connect(&m_engineMan.GetEngine(), SIGNAL(NotifyEngineCrashed()), this, SLOT(_engine_crashed()));
}

EngineControl::~EngineControl()
{
    delete ui;
}

void EngineControl::Go()
{
    m_engineMan.GetEngine().SetPosition(m_board->ToFEN());

    IEngine::ThinkParams p;
    p.MoveTime = ui->spin_thinkTime->value();
    p.Nodes = ui->spin_nodes->value();
    p.Depth = ui->spin_depth->value();
    p.Mate = ui->spin_mate->value();
    m_engineMan.GetEngine().StartThinking(p);
}

void EngineControl::Stop()
{
    m_engineMan.GetEngine().StopThinking();
}

void EngineControl::_msg_rx(const QByteArray &line)
{
    ui->tb_engineLog->append(line);
}

void EngineControl::_engine_crashed()
{
    ui->tb_engineLog->append("*** ENGINE CRASHED ***");
}


END_NAMESPACE_GKCHESS1;
