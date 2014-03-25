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
#include "gkchess_board.h"
#include "gkchess_pgn_move_data.h"
#include "gkchess_movedata.h"
#include "gutil_map.h"
#include <QObject>

namespace GKChess{


/** Describes the game logic for standard chess. */
class GameLogic :
        public QObject,
        public IGameLogic
{
    Q_OBJECT

    Board m_board;

    // These facilitate fast piece lookups
    GUtil::Map<Piece::PieceTypeEnum, ISquare const *> m_whitePieceIndex;
    GUtil::Map<Piece::PieceTypeEnum, ISquare const *> m_blackPieceIndex;

public:

    explicit GameLogic(QObject * = 0);
    virtual ~GameLogic();

    /** Returns a list of squares occupied by the type of piece specified.
     *  The list will be empty if it didn't find any.
     *
     *  This lookup is done in O(log(N)) time, where N is the number of different types of pieces.
    */
    GUtil::Vector<ISquare const *> FindPieces(const Piece &) const;

    /** Determines if the move is valid, according to the rules of standard chess.
     *  This is virtual to allow you to customize your own move validation if you like.
    */
    virtual MoveValidationEnum ValidateMove(const MoveData &) const;

    /** \name IGameLogic interface
     *  \{
    */
    virtual AbstractBoard const &GetBoard() const;
    virtual void SetupNewGame(SetupTypeEnum = StandardChess);
    virtual void Move(const PGN_MoveData &);
    virtual void Move(const MoveData &);
    /** \} */


private:

    /** Maps a MoveData object to our own move_data type. */
    MoveData _translate_move_data(const PGN_MoveData &);

    void _move(const MoveData &, bool reverse = false);

};


}


/** Defined for your convenience, a string that represents the initial chess position
 *  in Forsyth-Edwards notation. You can pass this to the PGN parser to generate a board
 *  object.
*/
#define FEN_STANDARD_CHESS_STARTING_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


#endif // GKCHESS_GAMELOGIC_H
