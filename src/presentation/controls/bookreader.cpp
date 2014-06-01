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
#include "gkchess_board.h"
#include "gkchess_polyglotreader.h"
#include "gutil_pluginutils.h"
#include "gutil_persistentdata.h"
#include <QFileDialog>
#include <QTableWidgetItem>
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(QT);

#define SETTING_LAST_BOOK "bookreader_last_open_book"

NAMESPACE_GKCHESS1(UI);


BookReader::BookReader(ObservableBoard &b, PersistentData *pd, QWidget *parent)
    :QWidget(parent),
      ui(new Ui::BookReader),
      m_board(b),
      m_settings(pd),
      i_bookreader(PluginUtils::LoadPlugin<IBookReader>(m_pl, "polyglotReaderPlugin"))
{
    ui->setupUi(this);
    ui->btn_validate->hide();
    ui->prg_validation->hide();
    ui->prg_validation->setRange(0, 100);

    connect(&b, SIGNAL(NotifyPieceMoved(const GKChess::MoveData &)),
            this, SLOT(board_position_changed()));
    connect(&b, SIGNAL(NotifySquareUpdated(const GKChess::Square &)),
            this, SLOT(board_position_changed()));
    connect(&b, SIGNAL(NotifyBoardReset()),
            this, SLOT(board_position_changed()));

    if(m_settings && m_settings->Contains(SETTING_LAST_BOOK)){
        ui->lineEdit->setText(m_settings->Value(SETTING_LAST_BOOK).toString());
        file_selected();
    }
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

void BookReader::CloseFile()
{
    if(!i_bookreader)
        return;

    if(i_bookreader->IsBookOpen())
    {
        i_bookreader->CloseBook();
        i_bookreader = 0;
        // Don't delete it, because the QPlugin abstraction takes care of it for us
    }
}

void BookReader::file_selected()
{
    QString filename = ui->lineEdit->text().trimmed();

    if(i_bookreader)
    {
        // If the filename didn't change, then return
        if(filename == i_bookreader->GetBookFilename())
            return;

        i_bookreader->CloseBook();
        ui->btn_validate->hide();
        ui->tableWidget->setRowCount(0);
    }

    if(!filename.isEmpty())
    {
        try{
            i_bookreader->OpenBook(filename.toUtf8().constData());
        } catch(...) {
            return;
        }

        ui->btn_validate->show();
    }
    board_position_changed();

    // Remember the last book
    if(m_settings)
        m_settings->SetValue(SETTING_LAST_BOOK, i_bookreader->GetBookFilename());
}

void BookReader::validate_file()
{
    if(!i_bookreader || !i_bookreader->IsBookOpen())
        return;

    ui->prg_validation->show();
    try
    {
        i_bookreader->ValidateBook(this);
    } catch(...){
        ui->prg_validation->hide();
        throw;
    }
    ui->prg_validation->hide();
    ui->btn_validate->hide();
}

void BookReader::OnValidationProgressUpdate(int p)
{
    if(p < 100){
        ui->prg_validation->show();
        ui->prg_validation->setValue(p);
    }
    else
        ui->prg_validation->hide();
}

void BookReader::board_position_changed()
{
    if(!i_bookreader)
        return;

    String s = m_board.ToFEN();
    Vector<IBookReader::Move> moves = i_bookreader->LookupMoves(s);
    int row = 0;

    ui->tableWidget->clearContents();
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
