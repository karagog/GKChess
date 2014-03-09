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
#include "gkchess_move_data.h"

NAMESPACE_GKCHESS;

class AbstractClock;


/** Describes the game logic for standard chess. */
class GameLogic
{
    Board m_board;
    Piece::AllegienceEnum m_currentTurn;

public:

    /** You must give the game logic a board to play on. It will not own the board. */
    explicit GameLogic();
    virtual ~GameLogic();

    /** Returns the game board. */
    Board const &GetBoard() const{ return m_board; }

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
    virtual void SetupNewGame(SetupTypeEnum = StandardChess);

    /** Moves based on a MoveData object, which is created from the PGN parser. */
    void Move(const MoveData &);

    /** Causes the last move to be undone. */
    virtual void Undo();

    /** Causes the last move that was undone to be redone. */
    virtual void Redo();

    /** Returns the allegience whose turn it is. */
    Piece::AllegienceEnum WhoseTurn() const;


    /** Encodes the ways a move can be validated. */
    enum MoveValidationEnum
    {
        /** Means the move is valid. */
        ValidMove = 0,

        /** Invalid move because pieces can't move that way according to the rules of chess.
         *  (i.e. a pawn moving backwards).
        */
        InvalidTechnical = 1,

        /** Invalid because you would be leaving the king in check. */
        InvalidCheck = 2,

        /** The source square does not have a piece on it. */
        InvalidEmptySquare = 3,


        /** If you are extending this class for your own custom rules then you'll base
         *  your validation types with this value.
        */
        CustomInvalidMoveOffset = 100
    };

    /** Determines if the move is valid, according to the rules of standard chess. */
    virtual MoveValidationEnum ValidateMove(const Square &source, const Square &destination) const;


    /** Returns a list of squares that are valid for the given square and piece. */
    ::GUtil::DataObjects::Vector<Square const *> GetPossibleMoves(const Square &, const Piece &) const;


protected:

    /** Describes all the data we need to remember each move. */
    struct move_data_t
    {
        Square *Source, *Destination;

        int CastleType;

        GUtil::Utils::SharedSmartPointer<Piece> PieceMoved;
        GUtil::Utils::SharedSmartPointer<Piece> PieceCaptured;
        GUtil::Utils::SharedSmartPointer<Piece> PiecePromoted;

        move_data_t();
    };

    /** This function actually carries out the move. */
    virtual void move_protected(const move_data_t &);


private:

    ::GUtil::DataObjects::Vector<move_data_t> m_moveHistory;
    GINT32 m_moveHistoryIndex;

    /** Maps a MoveData object to our own move_data type. */
    move_data_t _translate_move_data(const MoveData &) const;

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_GAMELOGIC_H
