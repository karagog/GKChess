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

#include "chess960generatorcontrol.h"
#include "ui_chess960generator.h"
#include "gkchess_chess960.h"
#include <QClipboard>
USING_NAMESPACE_GKCHESS;

NAMESPACE_GKCHESS1(UI);


Chess960Generator::Chess960Generator(QWidget *parent)
    :QDialog(parent),
      ui(new Ui::Chess960Generator)
{
    ui->setupUi(this);
    Generate();
}

Chess960Generator::~Chess960Generator()
{
    delete ui;
}

void Chess960Generator::Generate(int idx)
{
    if(m_ignoreSpinbox)
        return;

    if(idx == -1){
        m_fen = Chess960::GetRandomStartingPosition(&idx).ToQString();
    }
    else if(0 <= idx && idx < 960){
        m_fen = Chess960::GetStartingPosition(idx).ToQString();
    }
    ui->lbl_fen->setText(m_fen);

    m_ignoreSpinbox = true;
    ui->spin_posNumber->setValue(idx);
    m_ignoreSpinbox = false;
}

void Chess960Generator::CopyToClipboard()
{
    if(!m_fen.isEmpty() && QApplication::clipboard())
        QApplication::clipboard()->setText(m_fen);
}


END_NAMESPACE_GKCHESS1;
