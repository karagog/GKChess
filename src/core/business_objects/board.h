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

#ifndef GKCHESS_BOARD_H
#define GKCHESS_BOARD_H

#include "gkchess_abstractboard.h"

namespace GKChess{


/** The implementation for the GKChess board.
 *  See AbstractBoard for the interface description.
 * \sa AbstractBoard
*/
class Board :
        public AbstractBoard
{
    Q_OBJECT
    void *d;
public:

    explicit Board(QObject *parent = 0);
    virtual ~Board();


    /** \name AbstractBoard interface
     *  \{
    */
    virtual void SetPiece(const Piece &, int column, int row);
    virtual Square const &GetSquare(int column, int row) const;
    /** \}*/

};


}

#endif // GKCHESS_BOARD_H
