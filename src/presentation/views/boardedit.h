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

#ifndef GKCHESS_BOARDEDIT_H
#define GKCHESS_BOARDEDIT_H

#include "boardview.h"

namespace GKChess{ namespace UI{


class EditableBoardModel;


/** A board view that lets you modify the board. */
class BoardEdit :
        public BoardView
{
    Q_OBJECT

    QModelIndex m_activeSquare;
    bool m_dragging;

public:

    BoardEdit(QWidget * = 0);

    /** You are restricted to only editable board models. */
    virtual void setModel(QAbstractItemModel *);

    EditableBoardModel *GetBoardModel() const;


protected:

    virtual void attempt_move(const QModelIndex &, const QModelIndex &);

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);

    virtual void paint_board(QPainter &, const QRect &);


private:

    void _update_cursor_at_point(const QPointF &);

};


}}

#endif // GKCHESS_BOARDEDIT_H
