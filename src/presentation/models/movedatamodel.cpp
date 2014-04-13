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

#include "movedatamodel.h"
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS1(UI);


MoveDataModel::MoveDataModel(QObject *parent)
    :QAbstractItemModel(parent)
{}

void MoveDataModel::InitFromPGN(const GUtil::Vector<PGN_MoveData> &)
{
    THROW_NEW_GUTIL_EXCEPTION(NotImplementedException);
}
    
int MoveDataModel::rowCount(const QModelIndex &i) const
{
    return _get_container_from_index(i)->Moves.Length();
}

int MoveDataModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant MoveDataModel::data(const QModelIndex &i, int role) const
{
    QVariant ret;
    MoveDataCache *md = _get_data_from_index(i);
    if(md)
    {
        switch((Qt::ItemDataRole)role)
        {
        case Qt::DisplayRole:
            //ret = md->Data.Text;
            break;
        default:
            break;
        }
    }
    return ret;
}

QModelIndex MoveDataModel::index(int r, int c, const QModelIndex &i) const
{
    return createIndex(r, c, _get_container_from_index(i)->Moves[r]);
}

QModelIndex MoveDataModel::parent(const QModelIndex &i) const
{
    QModelIndex ret;
    MoveDataCache *mc = _get_data_from_index(i);
    if(mc){
        if(mc->Parent){
            MoveDataCache *grandparent = mc->Parent->Parent;
            int row = mc->Parent->Data.PlyNumber - 1;
            if(grandparent)
                row -= grandparent->Data.PlyNumber;
            ret = createIndex(row, 0, mc->Parent);
        }
    }
    return ret;
}

MoveDataModel::MoveContainer *MoveDataModel::_get_container_from_index(const QModelIndex &i)
{
    return i.isValid() ? _get_data_from_index(i) : &m_rootContainer;
}

MoveDataModel::MoveContainer const *MoveDataModel::_get_container_from_index(const QModelIndex &i) const
{
    return i.isValid() ? _get_data_from_index(i) : &m_rootContainer;
}

MoveDataModel::MoveDataCache *MoveDataModel::_get_data_from_index(const QModelIndex &i)
{
    return i.isValid() ? reinterpret_cast<MoveDataCache *>(i.internalPointer()) : 0;
}


END_NAMESPACE_GKCHESS1;
