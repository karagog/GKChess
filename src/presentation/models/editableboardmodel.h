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


    /** \name QAbstractTableModel interface
     *  \{
    */
    virtual bool setData(const QModelIndex &, const QVariant &, int = Qt::EditRole);
    virtual bool dropMimeData(const QMimeData *, Qt::DropAction, int, int, const QModelIndex &);
    virtual Qt::ItemFlags flags(const QModelIndex &) const;
    virtual Qt::DropActions supportedDragActions() const;
    virtual Qt::DropActions supportedDropActions() const;
    /** \} */

};


}}

#endif // GKCHESS_EDITABLEBOARDMODEL_H
