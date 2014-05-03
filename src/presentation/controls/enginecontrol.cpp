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

NAMESPACE_GKCHESS1(UI);


EngineControl::EngineControl(QWidget *parent)
    :QWidget(parent),
      ui(new Ui::EngineControl)
{
    ui->setupUi(this);

    setEnabled(false);
}

void EngineControl::Initialize(UCI_Client *cli, Board *b)
{
    if(cli && b)
    {
        m_uci = cli;
        m_board = b;
        connect(cli, SIGNAL(MessageReceived(QByteArray)), this, SLOT(_msg_rx(QByteArray)));
        connect(cli, SIGNAL(NotifyEngineCrashed()), this, SLOT(_engine_crashed()));
        setEnabled(true);
    }

    GASSERT(cli && b);
}

void EngineControl::Uninitialize()
{
    if(IsInitialized())
    {
        disconnect(m_uci, SIGNAL(NotifyEngineCrashed()), this, SLOT(_engine_crashed()));
        disconnect(m_uci, SIGNAL(MessageReceived(QByteArray)), this, SLOT(_msg_rx(QByteArray)));
        m_uci = 0;
        m_board = 0;
        setEnabled(false);
    }
}

EngineControl::~EngineControl()
{
    delete ui;
}

void EngineControl::Go()
{
    if(IsInitialized())
    {
        m_uci->SetPosition(m_board->ToFEN());

        UCI_Client::GoParams p;
        p.MoveTime = ui->spin_thinkTime->value();
        p.Nodes = ui->spin_nodes->value();
        p.Depth = ui->spin_depth->value();
        p.Mate = ui->spin_mate->value();
        m_uci->Go(p);
    }
}

void EngineControl::Stop()
{
    if(IsInitialized())
    {
        m_uci->Stop();
    }
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
