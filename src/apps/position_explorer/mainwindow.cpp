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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gkchess_pgn_parser.h"
USING_NAMESPACE_GKCHESS;

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_boardModel(&m_board),
      m_iconFactory(":/gkchess/icons/default", Qt::white, Qt::gray)
{
    ui->setupUi(this);

    m_board.FromFEN(FEN_STANDARD_CHESS_STARTING_POSITION);
    m_boardModel.SetGameLogic(&m_gameLogic);

    ui->boardView->SetIconFactory(&m_iconFactory);

    ui->boardView->SetBoardModel(&m_boardModel);
    
    //ui->boardView->hide();

    m_iconFactory.ChangeColors(Qt::green, Qt::red);
}

MainWindow::~MainWindow()
{
    delete ui;
}
