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

#include "abstractboard.h"
#include "piece.h"

NAMESPACE_GKCHESS;


AbstractBoard::AbstractBoard(QObject *p)
    :QObject(p)
{}

AbstractBoard::~AbstractBoard()
{}


void AbstractBoard::Clear()
{
    for(int i = 0; i < ColumnCount(); ++i)
        for(int j = 0; j < RowCount(); ++j)
            SetPiece(Piece(), i, j);
}


END_NAMESPACE_GKCHESS;
