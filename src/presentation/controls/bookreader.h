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

#ifndef BOOKREADER_H
#define BOOKREADER_H

#include "gutil_smartpointer.h"
#include <QWidget>

namespace Ui {
class BookReader;
}

namespace GKChess{
  class Board;
  class ObservableBoard;
  class IBookReader;

namespace UI{


class BookReader :
        public QWidget
{
    Q_OBJECT
    GUtil::SmartPointer<Ui::BookReader> ui;
    Board &m_board;
    GKChess::IBookReader *i_bookreader;

public:

    explicit BookReader(GKChess::ObservableBoard &, QWidget *parent = 0);
    ~BookReader();


public slots:

    void SelectFile();
    void CloseFile();


private slots:

    void file_selected();
    void board_position_changed();

};


}}

#endif // BOOKREADER_H
