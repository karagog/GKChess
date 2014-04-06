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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gkchess_boardview.h"
#include "gkchess_gamelogic.h"
#include "gkchess_board.h"
#include "gkchess_coloredpieceiconfactory.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    GKChess::Board m_board;
    GKChess::GameLogic m_gameLogic;
    GKChess::UI::ColoredPieceIconFactory m_iconFactory;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //PROPERTY(HtmlFormatOptions, GKChess::UI::BoardView::HtmlFormattingOptions);

    //QString GetBoardHtml() const;

private slots:

    void _change_light_color();
    void _change_dark_color();
    void _change_light_piece_color();
    void _change_dark_piece_color();
    void _change_piece_size(int);
    void _change_square_size(int);
    void _change_border_thickness(int);
    void _dump_to_file();
    void _update();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
