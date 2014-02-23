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

#include "boardmodel.h"

NAMESPACE_GKCHESS1(UI);


BoardModel::BoardModel(Board *b, QObject *parent)
    :QAbstractTableModel(parent),
      m_board(b)
{}

int BoardModel::rowCount(const QModelIndex &i) const
{
    int ret(0);
    if(!i.isValid())
        ret = m_board->RowCount();
    return ret;
}

int BoardModel::columnCount(const QModelIndex &i) const
{
    int ret(0);
    if(!i.isValid())
        ret = m_board->ColumnCount();
    return ret;
}

QVariant BoardModel::data(const QModelIndex &i, int role) const
{
    QVariant ret;
    return ret;
}

QVariant BoardModel::headerData(int col, Qt::Orientation o, int role) const
{
    QVariant ret;
    return ret;
}


END_NAMESPACE_GKCHESS1;
