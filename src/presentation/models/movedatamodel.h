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

#ifndef GKCHESS_MOVEDATAMODEL_H
#define GKCHESS_MOVEDATAMODEL_H

#include "gkchess_pgn_move_data.h"
#include "gkchess_abstractboard.h"
#include "gutil_vector.h"
#include <QAbstractItemModel>

namespace GKChess{ namespace UI{


/** A model to display move data in its normally list form, but also tree form if necessary. */
class MoveDataModel :
        public QAbstractItemModel
{
    Q_OBJECT
    
    struct MoveDataCache;
    
    /** Contains a list of moves. */
    struct MoveContainer
    {
        GUtil::Vector<MoveDataCache *> Moves;
    };
    
    /** One cached move consists of the actual move data, a reference to
        the parent index and a list of other moves.
    */
    struct MoveDataCache : 
        public MoveContainer
    {
        /** A reference to my parent index. */
        MoveDataCache *Parent;
        
        /** The data held within an index. */
        MoveData Data;
        
        MoveDataCache(MoveDataCache *parent = 0) :Parent(parent){}
    };
    
    MoveContainer m_rootContainer;
    
    
public:

    explicit MoveDataModel(QObject * = 0);
    
    /** Initializes the model from a list of pgn moves. 
        These PGN moves are created by the PGN_Parser.
    */
    void InitFromPGN(const GUtil::Vector<PGN_MoveData> &);
    
    /** QAbstractItemModel interface
        \{
    */
    virtual int rowCount(const QModelIndex & = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &, int) const;
    virtual QModelIndex index(int, int, const QModelIndex & = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &) const;
    /** \} */
    
    
private:

    // This will always return a value, because even the root is a container
    MoveContainer *_get_container_from_index(const QModelIndex &);
    MoveContainer const *_get_container_from_index(const QModelIndex &) const;
    
    // This will always return a value except when the root index is passed
    static MoveDataCache *_get_data_from_index(const QModelIndex &);

};


}}

#endif // GKCHESS_MOVEDATAMODEL_H
