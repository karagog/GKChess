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

#include "movehistorycontrol.h"
#include "ui_movehistorycontrol.h"
#include "gkchess_board.h"

NAMESPACE_GKCHESS1(UI);


MoveHistoryControl::MoveHistoryControl(ObservableBoard &b, QWidget *parent)
    :QWidget(parent),
      m_board(b),
      m_recorder(b),
      ui(new Ui::MoveHistoryControl)
{
    ui->setupUi(this);
    connect(&m_recorder, SIGNAL(NotifyHistoryUpdated()), this, SLOT(history_updated()));
}

MoveHistoryControl::~MoveHistoryControl()
{
    delete ui;
}

void MoveHistoryControl::goto_first()
{

}

void MoveHistoryControl::go_forward()
{

}

void MoveHistoryControl::go_back()
{

}

void MoveHistoryControl::history_updated()
{
    // Refresh the text in the view
    ui->pgnView->clear();
    QTextCursor cur(ui->pgnView->document()->rootFrame());

    G_FOREACH_CONST(const MoveData &md, m_recorder.GetHistory()){
        if(1 == (md.PlyNumber & 0x1))
            cur.insertText(QString("%1. ").arg(md.PlyNumber / 2 + 1));
        cur.insertText(md.PGNData.ToString());
        cur.insertText(" ");
        if(0 == (md.PlyNumber & 0x1))
            cur.insertText(" ");
    }
}


END_NAMESPACE_GKCHESS1;
