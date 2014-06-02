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

#include "bookreadercontrol.h"
#include "ui_bookreadercontrol.h"
#include "gkchess_board.h"
#include "gkchess_polyglotreader.h"
#include "gkchess_movedata.h"
#include "gutil_persistentdata.h"
//#include "src/test/modeltest.h"
#include <QFileDialog>
#include <QTableWidgetItem>
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(QT);

#define SETTING_LAST_BOOK "bookreader_last_open_book"

NAMESPACE_GKCHESS1(UI);


BookReaderControl::BookReaderControl(ObservableBoard &b, PersistentData *pd, QWidget *parent)
    :QWidget(parent),
      ui(new Ui::BookReaderControl),
      m_board(b),
      m_settings(pd),
      m_bookModel(b)
{
    ui->setupUi(this);
    ui->treeView->setModel(&m_bookModel);

//    new ModelTest(&m_bookModel);

    if(m_settings && m_settings->Contains(SETTING_LAST_BOOK)){
        ui->lineEdit->setText(m_settings->Value(SETTING_LAST_BOOK).toString());
        file_selected();
    }
}

BookReaderControl::~BookReaderControl()
{}

void BookReaderControl::SelectFile()
{
    QString fn = QFileDialog::getOpenFileName(this, "Select Book", QString(), "*.bin");
    if(!fn.isEmpty()){
        ui->lineEdit->setText(fn);
        file_selected();
    }
}

void BookReaderControl::CloseFile()
{
    m_bookModel.SetBookFile("");
}

void BookReaderControl::file_selected()
{
    QString filename = ui->lineEdit->text().trimmed();

    m_bookModel.SetBookFile(filename);

    // Remember the last book
    if(m_settings)
        m_settings->SetValue(SETTING_LAST_BOOK, m_bookModel.GetBookFile());
}

void BookReaderControl::move_doubleClicked(const QModelIndex &ind)
{
    List<MoveData> lst;
    foreach(const QModelIndex &i, m_bookModel.GetAncestry(ind)){
        lst.Append(*m_bookModel.ConvertIndexToMoveData(i));
    }
    G_FOREACH_CONST(const MoveData &md, lst){
        m_board.Move(md);
    }
}

void BookReaderControl::OnValidationProgressUpdate(int p)
{
    GUTIL_UNUSED(p);
//    if(p < 100){
//        ui->prg_validation->show();
//        ui->prg_validation->setValue(p);
//    }
//    else
//        ui->prg_validation->hide();
}


END_NAMESPACE_GKCHESS1;
