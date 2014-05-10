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

#include "bookreader.h"
#include "ui_bookreader.h"
#include "gkchess_ibookreader.h"
#include "gkchess_board.h"
#include "gkchess_polyglotreader.h"
#include <QFileDialog>
//#include <QPluginLoader>
#include <QTableWidgetItem>
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS1(UI);

//#ifdef Q_OS_UNIX
//#define PLUGIN_PREFIX "lib"
//#define PLUGIN_SUFFIX ".so"
//#elif Q_OS_WIN
//#define PLUGIN_PREFIX ""
//#define PLUGIN_SUFFIX ".dll"
//#endif

BookReader::BookReader(ObservableBoard &b, QWidget *parent)
    :QWidget(parent),
      ui(new Ui::BookReader),
      m_board(b),
      i_bookreader(new PolyglotBookReader)
{
    ui->setupUi(this);

    connect(&b, SIGNAL(NotifyPieceMoved(const GKChess::MoveData &)),
            this, SLOT(board_position_changed()));
    connect(&b, SIGNAL(NotifySquareUpdated(const GKChess::Square &)),
            this, SLOT(board_position_changed()));
    connect(&b, SIGNAL(NotifyBoardReset()),
            this, SLOT(board_position_changed()));
}

BookReader::~BookReader()
{}

void BookReader::SelectFile()
{
    QString fn = QFileDialog::getOpenFileName(this, "Select Book", QString(), "*.bin");
    if(!fn.isEmpty()){
        ui->lineEdit->setText(fn);
        file_selected();
    }
}

void BookReader::file_selected()
{
    i_bookreader->OpenBook(ui->lineEdit->text().trimmed().toUtf8().constData());
    board_position_changed();
}

void BookReader::board_position_changed()
{
    String s = m_board.ToFEN();
    Vector<IBookReader::Move> moves = i_bookreader->LookupMoves(s);
    ui->tableWidget->clearContents();
    int row = 0;
    ui->tableWidget->setRowCount(moves.Length());
    G_FOREACH_CONST(IBookReader::Move const &m, moves){
        QTableWidgetItem *item1 = new QTableWidgetItem(m.Text.ToQString(), QVariant::String);
        QTableWidgetItem *item2 = new QTableWidgetItem;
        item2->setData(Qt::DisplayRole, m.Weight);
        item2->setData(Qt::TextAlignmentRole, Qt::AlignCenter);
        item1->setData(Qt::TextAlignmentRole, Qt::AlignCenter);
        ui->tableWidget->setItem(row, 0, item1);
        ui->tableWidget->setItem(row, 1, item2);
        ++row;
    }
}


END_NAMESPACE_GKCHESS1;
