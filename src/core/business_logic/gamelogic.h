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

#include "gkchess_board.h"

NAMESPACE_GKCHESS;


/** Describes the game logic for standard chess. */
class GameLogic
{
    Board m_board;
    Piece::AllegienceEnum m_currentMove;

public:
    GameLogic();

    /** Returns the game board. */
    inline Board const &GetBoard() const{ return m_board; }

    /** Describes different ways the board could be set up. */
    enum SetupTypeEnum
    {
        /** Causes the board to be cleared of all pieces. */
        Empty = 0,

        /** A standard game of chess. */
        StandardChess = 1,

        /** You can create your own custom board setups starting from this offset. */
        CustomSetupOffset = 100
    };

    /** Sets up the board for a new game. */
    void SetupNewGame(SetupTypeEnum = StandardChess);


    /** Returns a list of squares that are valid for the given square.
     *  If there is no piece on the square, or the piece has no moves, then an empty list is returned.
    */
    ::GUtil::DataObjects::Vector<Square *> GetPossibleMoves(const Square &) const;

    /** Attempts to move the piece at the source square to the destination.
     *  If the move is not possible (according to the rules of standard chess)
     *  it will throw an exception.
    */
    void Move(Square &source, Square &destination);

    /** Causes the last move to be undone. */
    void Undo();

    /** Causes the last move that was undone to be redone. */
    void Redo();


private:

    struct MoveData
    {
        Square *SourceSquare;
        Square *DestSquare;

        Piece *CapturedPiece;

        MoveData();
    };

    ::GUtil::DataObjects::Vector<MoveData> m_moveHistory;
    GINT32 m_moveHistoryIndex;

    void _execute_move(Square &source, Square &dest);
    MoveData const *_get_last_move() const;

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_GAMELOGIC_H
