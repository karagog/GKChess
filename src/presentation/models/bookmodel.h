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

#ifndef GKCHESS_BOOKMODEL_H
#define GKCHESS_BOOKMODEL_H

#include "gkchess_ibookreader.h"
#include "gkchess_board_movedata.h"
#include <QAbstractItemModel>
#include <QPluginLoader>

namespace GKChess{
class ObservableBoard;
namespace UI{


/** A model for navigating an opening book in a tree view.
 *  It lazy-loads each node.
*/
class BookModel :
        public QAbstractItemModel
{
    Q_OBJECT

    struct MoveDataCache;

    /** Contains a list of moves. */
    struct MoveDataContainer
    {
        QList<MoveDataCache> Moves;
        bool Loaded;
        MoveDataContainer() :Loaded(false) {}
    };

    /** One cached move consists of the actual move data, a reference to
        the parent index and a list of other moves.
    */
    struct MoveDataCache :
        public MoveDataContainer
    {
        /** A reference to my parent index. */
        MoveDataCache *Parent;

        /** The data held within an index. */
        MoveData Data;

        /** Stores the data from the book. */
        BookMove BookData;

        bool operator == (const MoveDataCache &o) const{ return this == &o; }

        explicit MoveDataCache(MoveDataCache *parent = 0) :Parent(parent){}
    };

    Board &m_board;
    QPluginLoader m_pl;
    IBookReader *i_bookReader;
    MoveDataContainer m_rootContainer;
public:

    explicit BookModel(ObservableBoard &, QObject *parent = 0);

    /** Opens the book file at the given file path. */
    bool SetBookFile(const QString &filename);

    /** Returns the current book file. */
    QString GetBookFile() const;

    /** Returns all ancestors of the given index, including the index */
    QModelIndexList GetAncestry(const QModelIndex &) const;

    /** Returns the move data at the given index.  If the index is invalid
     *  this will return a null pointer.
    */
    MoveData const *ConvertIndexToMoveData(const QModelIndex &) const;

    /** \name QAbstractItemModel interface
     *  \{
    */

    bool hasChildren(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    void fetchMore(const QModelIndex &);
    bool canFetchMore(const QModelIndex &parent) const;

    /** \} */


private slots:

    void _board_position_changed();


private:

    MoveDataCache *_get_data_from_index(const QModelIndex &) const;
    MoveDataContainer const *_get_children_of_index(const QModelIndex &) const;
    MoveDataContainer *_get_children_of_index(const QModelIndex &);

    void _get_ancestry_helper(QModelIndexList &, const QModelIndex &) const;

};


}}

#endif // GKCHESS_BOOKMODEL_H
