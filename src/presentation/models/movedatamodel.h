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

#include "gkchess_movedata.h"
#include <QAbstractItemModel>

namespace GKChess{ namespace UI{


/** A model to display move data in its normally list form, but also tree form if necessary. */
class MoveDataModel :
        public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit MoveDataModel(QObject *parent = 0);

};


}}

#endif // GKCHESS_MOVEDATAMODEL_H
