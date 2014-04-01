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

#ifndef GKCHESS_EDITABLEBOARDMODEL_H
#define GKCHESS_EDITABLEBOARDMODEL_H

#include "boardmodel.h"

namespace GKChess{ namespace UI{


/** The editable board model allows direct editing of the chess board through the
 *  view.  You can spawn any piece wherever you want and you can move them around
 *  without following the rules of chess.
*/
class EditableBoardModel :
        public BoardModel
{
    Q_OBJECT
    AbstractBoard *m_board;
public:

    /** Constructs an editable board model from a non-const board object. */
    explicit EditableBoardModel(AbstractBoard *, QObject * = 0);

    /** Moves the piece at the source index to the dest index.
     *
     *  The base implementation accepts all moves without validation, and whatever piece at the
     *  destination will be overwritten by the one from source.
    */
    virtual void MovePiece(const QModelIndex &source, const QModelIndex &);

    /** Returns true if the move is allowable.  The base implementation always returns true. */
    virtual bool ValidateMove(const QModelIndex &source, const QModelIndex &dest);


    /** \name QAbstractTableModel interface
     *  \{
    */
    virtual QVariant data(const QModelIndex &, int) const;
    virtual bool setData(const QModelIndex &, const QVariant &, int = Qt::EditRole);
    virtual bool dropMimeData(const QMimeData *, Qt::DropAction, int, int, const QModelIndex &);
    virtual Qt::ItemFlags flags(const QModelIndex &) const;
    virtual Qt::DropActions supportedDragActions() const;
    virtual Qt::DropActions supportedDropActions() const;
    /** \} */

};


}}

#endif // GKCHESS_EDITABLEBOARDMODEL_H
