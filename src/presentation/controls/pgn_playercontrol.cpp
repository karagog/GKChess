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

NAMESPACE_GKCHESS1(UI);


PGN_PlayerControl::PGN_PlayerControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PGN_PlayerControl)
{
    ui->setupUi(this);
}

PGN_PlayerControl::~PGN_PlayerControl()
{
    delete ui;
}


END_NAMESPACE_GKCHESS1;
