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

#include <QAbstractTableModel>

namespace GKChess{ 
    class AbstractBoard;
    class ISquare;

namespace UI{


/** Describes a data model for a chess board. This is a readonly model, so you can
 *  only display boards with it.
 *
 *  This can be used with Qt's model-view framework.
*/
class BoardModel :
        public QAbstractTableModel
{
    Q_OBJECT

    AbstractBoard const *m_board;
public:

    /** You must give the model a reference to a chessboard
     *  object. It must exist at least for the lifetime of
     *  this object.
    */
    explicit BoardModel(AbstractBoard const *, QObject *parent = 0);

    /** Returns a reference to the square at the given index.
     *  It will return 0 on errors.
    */
    ISquare const *ConvertIndexToSquare(const QModelIndex &) const;

    /** Returns the model index corresponding to the given square. */
    QModelIndex ConvertSquareToIndex(const ISquare &) const;


    /** \name QAbstractTableModel interface
     *  \{
    */
    virtual int rowCount(const QModelIndex & = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &, int) const;
    virtual QVariant headerData(int, Qt::Orientation, int) const;
    virtual Qt::ItemFlags flags(const QModelIndex &) const;

    virtual QMimeData *mimeData(const QModelIndexList &) const;
    virtual QStringList mimeTypes() const;
    /** \} */


private slots:

    void _square_updated(int, int);

};


}}


/** Defines the mimetype used for dragging and dropping pieces. You can use this
 *  to drop pieces onto the board.
 *
 *  The data is an ascii string with the following format:   <piece>:<column>,<row>
 *  Where the piece is a single char used in FEN notation and the column and row are
 *  0-based indices describing the source of the piece. Leave the source info blank
 *  if it describes a new piece.
*/
#define MIMETYPE_GKCHESS_PIECE "gkchess/piece"


#endif // GKCHESS_BOARDMODEL_H