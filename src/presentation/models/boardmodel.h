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

#ifndef GKCHESS_BOARDMODEL_H
#define GKCHESS_BOARDMODEL_H

#include "gkchess_board.h"
#include <QAbstractTableModel>

namespace GKChess{ namespace UI{


/** Describes a data model for a chess board.
 *
 *  This can be used with Qt's model-view framework, specifically
 *  with a BoardView.
*/
class BoardModel :
        public QAbstractTableModel
{
    Q_OBJECT

    Board *m_board;

public:

    /** You must give the model a reference to a chessboard
     *  object. It must exist at least for the lifetime of
     *  this object.
    */
    explicit BoardModel(Board *, QObject *parent = 0);

    /** These are extra data roles we need to represent a chessboard. */
    enum ChessDataRoles
    {
        /** Returns a bool whether the square is able to be en passant captured.
         *  This will return true on squares that have just been passed by
         *  a pawn that moved 2 squares on the previous move.
        */
        EnPassantEnabled = Qt::UserRole + 0,

        /** Returns a bool whether the king can castle to this square. */
        CastleEnabled = Qt::UserRole + 1
    };



    /** \name QAbstractTableModel interface
     *  \{
    */
    virtual int rowCount(const QModelIndex &) const;
    virtual int columnCount(const QModelIndex &) const;
    virtual QVariant data(const QModelIndex &, int) const;
    virtual QVariant headerData(int, Qt::Orientation, int) const;
    /** \} */


};


}

#endif // GKCHESS_BOARDMODEL_H
