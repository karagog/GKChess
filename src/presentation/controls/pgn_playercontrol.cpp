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
#include "gkchess_globals.h"
#include "gkchess_pgn_player.h"
#include "gkchess_pgn_parser.h"
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS1(UI);


struct d_t
{
    Ui::PGN_PlayerControl *ui;
    PGN_Player *player;
};

PGN_PlayerControl::PGN_PlayerControl(Board *b, QWidget *parent)
    :QWidget(parent)
{
    G_D_INIT();
    G_D;
    d->ui = new Ui::PGN_PlayerControl;
    d->ui->setupUi(this);
    setEnabled(false);
    d->player = new PGN_Player(b);
}

PGN_PlayerControl::~PGN_PlayerControl()
{
    G_D;
    delete d->ui;
    delete d->player;
    G_D_UNINIT();
}

void PGN_PlayerControl::LoadPGN(const String &s)
{
    G_D;
    d->player->LoadPGN(s);
    const PGN_GameData &pgd = d->player->GetGameData();

    if(pgd.Tags.Contains("white") && pgd.Tags.Contains("black"))
        d->ui->lbl_title->setText(QString("%1 vs. %2")
                                  .arg(pgd.Tags.At("white"))
                                  .arg(pgd.Tags.At("black")));

    if(pgd.Tags.Contains("date"))
        d->ui->lbl_date->setText(QString("Date: %1").arg(pgd.Tags.At("date")));
    if(pgd.Tags.Contains("result"))
        d->ui->lbl_result->setText(QString("Result: %1").arg(pgd.Tags.At("result")));

    d->ui->pgn_view->setText(d->player->GetPGNText());
    setEnabled(true);
}

void PGN_PlayerControl::Clear()
{
    G_D;
    d->player->Clear();
    d->ui->pgn_view->clear();
    d->ui->lbl_title->clear();
    d->ui->lbl_date->clear();
    d->ui->lbl_result->clear();
    setEnabled(false);
}

void PGN_PlayerControl::GotoNext()
{
    G_D;
    d->player->Next();
}

void PGN_PlayerControl::GotoPrevious()
{
    G_D;
    d->player->Previous();
}

void PGN_PlayerControl::GotoFirst()
{
    G_D;
    d->player->First();
}

void PGN_PlayerControl::GotoLast()
{
    G_D;
    d->player->Last();
}

void PGN_PlayerControl::GotoIndex(int)
{

}


END_NAMESPACE_GKCHESS1;
