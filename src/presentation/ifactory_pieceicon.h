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

#ifndef GKCHESS_IPIECEGKCHESS_ICONFACTORY_H
#define GKCHESS_IPIECEGKCHESS_ICONFACTORY_H

#include <QObject>
#include <QIcon>

namespace GKChess{
    class Piece;
namespace UI{


/** Declares an interface that generates icons for pieces. 
    
    The benefit of this interface is that all the views can use
    the same piece icons and nobody will care where they came from.
*/
class IFactory_PieceIcon :
        public QObject
{
    Q_OBJECT
public:

    explicit IFactory_PieceIcon(QObject *p = 0):QObject(p){}

    /** Returns a QIcon that corresponds to the given piece. */
    virtual QIcon GetIcon(const Piece &) = 0;


signals:

    /** This signal emits when the set of icons gets updated. */
    void NotifyIconsUpdated();
    
};


}}

#endif // GKCHESS_IPIECEGKCHESS_ICONFACTORY_H
