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
#include "gkchess_ibookreader.h"
#include "gkchess_bookmodel.h"
#include <QWidget>
#include <QPluginLoader>

namespace Ui {
class BookReaderControl;
}

namespace GUtil{ namespace QT{
class PersistentData;
}}

namespace GKChess{
  class Board;
  class ObservableBoard;

namespace UI{


class BookReaderControl :
        public QWidget,
        private IBookReader::IValidationProgressObserver
{
    Q_OBJECT
    GUtil::SmartPointer<Ui::BookReaderControl> ui;
    Board &m_board;
    GUtil::QT::PersistentData *m_settings;
    GKChess::UI::BookModel m_bookModel;

public:

    /** If you pass a persistent data object, then we will be able to remember the last book you had open. */
    explicit BookReaderControl(GKChess::ObservableBoard &, GUtil::QT::PersistentData * = 0, QWidget *parent = 0);
    ~BookReaderControl();


public slots:

    void SelectFile();
    void CloseFile();


private slots:

    void file_selected();


private:

    void OnValidationProgressUpdate(int);

};


}}

#endif // BOOKREADER_H
