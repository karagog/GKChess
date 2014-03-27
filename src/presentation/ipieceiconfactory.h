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

#ifndef GKCHESS_IPIECEICONFACTORY_H
#define GKCHESS_IPIECEICONFACTORY_H

#include <QIcon>

namespace GKChess{
    class Piece;
namespace UI{


/** Declares an interface that generates icons for pieces. 
    
    The benefit of this interface is that all the views can use
    the same piece icons and nobody will care where they came from.
*/
class IPieceIconFactory
{
public:

    /** Returns a QIcon that corresponds to the given piece. */
    virtual QIcon GetIcon(const Piece &) const = 0;
    
};


}}

#endif // GKCHESS_IPIECEICONFACTORY_H
