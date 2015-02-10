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

#include "pgn_playercontrol.h"
#include "ui_pgn_playercontrol.h"
#include <gkchess_common.h>
#include "gkchess_pgn_player.h"
#include "gkchess_pgn_parser.h"
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GUTIL;


NAMESPACE_GKCHESS1(UI);


PGN_PlayerControl::PGN_PlayerControl(Board &b, QWidget *parent)
    :QWidget(parent),
      ui(new Ui::PGN_PlayerControl),
      player(new PGN_Player(b))
{
    ui->setupUi(this);
    setEnabled(false);
}

PGN_PlayerControl::~PGN_PlayerControl()
{
    delete player;
    delete ui;
}

void PGN_PlayerControl::LoadPGN(const String &s)
{
    player->LoadPGN(s);
    const PGN_GameData &pgd = player->GetGameData();

    if(pgd.Tags.contains("white") && pgd.Tags.contains("black"))
        ui->lbl_title->setText(QString("%1 vs. %2")
                                  .arg(pgd.Tags["white"])
                                  .arg(pgd.Tags["black"]));

    if(pgd.Tags.contains("date"))
        ui->lbl_date->setText(QString("Date: %1").arg(pgd.Tags["date"]));
    if(pgd.Tags.contains("result"))
        ui->lbl_result->setText(QString("Result: %1").arg(pgd.Tags["result"]));

    setEnabled(true);
}

void PGN_PlayerControl::Clear()
{
    player->Clear();
    ui->lbl_title->clear();
    ui->lbl_date->clear();
    ui->lbl_result->clear();
    setEnabled(false);
}

void PGN_PlayerControl::GotoNext()
{
    player->Next();
}

void PGN_PlayerControl::GotoPrevious()
{
    player->Previous();
}

void PGN_PlayerControl::GotoFirst()
{
    player->First();
}

void PGN_PlayerControl::GotoLast()
{
    player->Last();
}

void PGN_PlayerControl::GotoIndex(int)
{

}


END_NAMESPACE_GKCHESS1;
