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
#include "gkchess_manage_engines.h"
#include "gkchess_board.h"
#include "gkchess_uci_client.h"
#include "gkchess_enginesettings.h"
#include "gkchess_uiglobals.h"
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GUTIL1(QT);
USING_NAMESPACE_GUTIL;

#define NO_ENGINE_TEXT "(none)"

NAMESPACE_GKCHESS1(UI);


EngineControl::EngineControl(Board *b, EngineSettings *settings, PersistentData *appSettings, QWidget *parent)
    :QWidget(parent),
      m_board(b),
      m_settings(settings),
      m_appSettings(appSettings),
      m_suppressUpdate(false),
      ui(new Ui::EngineControl)
{
    ui->setupUi(this);

    _engines_updated();

//    if(settings->GetEngineList().count() > 0)
//        _engine_selection_changed(ui->cmb_engine->currentText());

    connect(m_settings, SIGNAL(NotifyEnginesUpdated()),
            this, SLOT(_engines_updated()));
    connect(ui->cmb_engine, SIGNAL(activated(int)),
            this, SLOT(_engine_selection_activated(int)));
    connect(ui->cmb_engine, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(_engine_selection_changed(QString)));
}

EngineControl::~EngineControl()
{
    delete ui;
}

void EngineControl::_engines_updated()
{
    m_engineList = m_settings->GetEngineList();
    m_engineList.sort();

    // Remove engines from the combo box first
    QStringList combo_list;
    for(int i = ui->cmb_engine->count() - 1; i >= 0; --i){
        QString engine_name = ui->cmb_engine->itemText(i);
        if(m_engineList.contains(engine_name))
            combo_list.append(engine_name);
        else
            ui->cmb_engine->removeItem(i);
    }

    // Then add new ones
    if(0 < m_engineList.length()){
        int i = 0;
        G_FOREACH_CONST(const String &k, m_engineList){
            int indx = combo_list.indexOf(k);
            if(-1 == indx)
                ui->cmb_engine->insertItem(i, k, i);
            ++i;
        }
        ui->wdg_control->setEnabled(true);
    }
    else{
        ui->cmb_engine->addItem(tr(NO_ENGINE_TEXT), -1);
        ui->wdg_control->setEnabled(false);
    }

    const QString last_engine = m_appSettings->Value(GKCHESS_SETTING_LAST_ENGINE_USED).toString();
    int indx = 0;
    if(!last_engine.isEmpty()){
        indx = ui->cmb_engine->findText(last_engine);
        if(-1 == indx)
            indx = 0;
    }
    ui->cmb_engine->setCurrentIndex(indx);
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

void EngineControl::_engine_selection_activated(int indx)
{
    if(0 == indx && ui->cmb_engine->currentText() == NO_ENGINE_TEXT){
        // Open the engine settings manager
        ManageEngines mng(m_settings, m_appSettings, this);
        mng.exec();
    }
}

void EngineControl::_engine_selection_changed(const QString &engine_name)
{
    if(engine_name.isEmpty())
        return;

    if(!m_engineMan || m_engineMan->GetEngineName() != engine_name){
        m_engineMan = new EngineManager(ui->cmb_engine->currentText(), m_settings);
        connect(&m_engineMan->GetEngine(), SIGNAL(MessageReceived(QByteArray)), this, SLOT(_msg_rx(QByteArray)));
        connect(&m_engineMan->GetEngine(), SIGNAL(NotifyEngineCrashed()), this, SLOT(_engine_crashed()));
        connect(&m_engineMan->GetEngine(), SIGNAL(BestMove(QByteArray,QByteArray)),
                this, SLOT(Stop()));

        // Whenever we switch engines, remember the last one we used
        m_appSettings->SetValue(GKCHESS_SETTING_LAST_ENGINE_USED, engine_name);
    }
    else{
        m_engineMan->ApplySettings();
    }
    ui->wdg_control->setEnabled(true);
}


END_NAMESPACE_GKCHESS1;
