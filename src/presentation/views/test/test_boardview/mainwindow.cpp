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
#include "gkchess_gamelogic.h"
#include "gkchess_editableboardmodel.h"
#include "gkchess_pgn_parser.h"
#include <QFile>
#include <QColorDialog>
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GKCHESS1(UI);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_board(PGN_Parser::FromX_FEN(FEN_STANDARD_CHESS_STARTING_POSITION)),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->sldr_borderSize->setValue(GetHtmlFormatOptions().BorderSize);
    ui->sldr_pieceSize->setValue(GetHtmlFormatOptions().PieceSize);
    ui->sldr_squareSize->setValue(GetHtmlFormatOptions().SquareSize);

    //m_game.SetupNewGame();

    ui->tableView->setModel(new EditableBoardModel(&m_board));

    _update();
}

QString MainWindow::GetBoardHtml() const
{
    return BoardView::GenerateHtml(m_board, GetHtmlFormatOptions());
}

void MainWindow::_change_light_color()
{
    QColorDialog dlg(GetHtmlFormatOptions().LightSquareColor);
    if(QDialog::Accepted == dlg.exec())
    {
        //m_game.SetLightColor(dlg.selectedColor());
        _p_HtmlFormatOptions.LightSquareColor = dlg.selectedColor();
        _update();
    }
}

void MainWindow::_change_dark_color()
{
    QColorDialog dlg(GetHtmlFormatOptions().DarkSquareColor);
    if(QDialog::Accepted == dlg.exec())
    {
        //m_game.SetDarkColor(dlg.selectedColor());
        _p_HtmlFormatOptions.DarkSquareColor = dlg.selectedColor();
        _update();
    }
}

void MainWindow::_change_piece_color()
{
    QColorDialog dlg(GetHtmlFormatOptions().PieceColor);
    if(QDialog::Accepted == dlg.exec())
    {
        //m_game.SetPieceColor(dlg.selectedColor());
        _p_HtmlFormatOptions.PieceColor = dlg.selectedColor();
        _update();
    }
}

void MainWindow::_change_piece_size(int s)
{
    //m_game.SetPieceSize(s);
    _p_HtmlFormatOptions.PieceSize = s;
    _update();
}

void MainWindow::_change_square_size(int s)
{
    _p_HtmlFormatOptions.SquareSize = s;
    _update();
}

void MainWindow::_change_border_thickness(int s)
{
    _p_HtmlFormatOptions.BorderSize = s;
    _update();
}

#define DUMP_FILENAME "board.html"

void MainWindow::_dump_to_file()
{
    // Write the board to a file so we can see it in a web browser
    QFile f(DUMP_FILENAME);
    f.open(QFile::WriteOnly | QFile::Truncate);
    f.write(GetBoardHtml().toUtf8());
    f.close();

    ui->statusBar->showMessage("Board dumped to " DUMP_FILENAME, 3000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::_update()
{
    ui->webView->setHtml(GetBoardHtml());
}
