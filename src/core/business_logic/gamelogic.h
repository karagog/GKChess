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

#ifndef GKCHESS_GAMELOGIC_H
#define GKCHESS_GAMELOGIC_H

#include "gkchess_igamelogic.h"
#include "gutil_map.h"
#include <QObject>

namespace GKChess{


/** Describes the game logic for standard chess. */
class GameLogic :
        public QObject,
        public IGameLogic
{
    Q_OBJECT
public:

    explicit GameLogic(QObject * = 0);
    virtual ~GameLogic();

    /** \name IGameLogic interface
     *  \{
    */
    virtual void SetupNewGame(AbstractBoard &, SetupTypeEnum = StandardChess);
    virtual AbstractBoard::MoveData GenerateMoveData(AbstractBoard const &, const ISquare &, const ISquare &, IPlayerResponse *) const;
    virtual AbstractBoard::MoveData GenerateMoveData(AbstractBoard const &, const PGN_MoveData &) const;
    virtual MoveValidationEnum ValidateMove(AbstractBoard const &, const ISquare &source, const ISquare &dest) const;
    virtual GUtil::Vector<ISquare const *> GetValidMovesForSquare(AbstractBoard const &, const ISquare &) const;
    /** \} */

};


}


/** Defined for your convenience, a string that represents the initial chess position
 *  in Forsyth-Edwards notation. You can pass this to the PGN parser to generate a board
 *  object.
*/
#define FEN_STANDARD_CHESS_STARTING_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


#endif // GKCHESS_GAMELOGIC_H
