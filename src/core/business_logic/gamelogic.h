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
#include "gkchess_pgn_move_data.h"
#include "gutil_map.h"
#include <QObject>

namespace GKChess{

class AbstractClock;


/** Describes the game logic for standard chess. */
class GameLogic :
        public QObject
{
    Q_OBJECT

    Board m_board;
    Piece::AllegienceEnum m_currentTurn;

public:

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

    /** Holds all the information we need to do and undo a move*/
    struct MoveData
    {
        /** The starting square.  If the move was a castle this will be null. */
        ISquare const *Source;

        /** The ending square.  If the move was a castle this will be null. */
        ISquare const *Destination;

        /** The type of castle is either 0=No Casle, 1=Castle Normal, -1=Castle Queenside. */
        enum CastleTypeEnum
        {
            NoCastle = 0,
            CastleNormal = 1,
            CastleQueenside = -1
        }
        CastleType;

        /** The piece being moved. */
        Piece PieceMoved;

        /** The captured piece, if any. If this is type NoPiece then the
         *  move did not involve a capture.
        */
        Piece PieceCaptured;

        /** The piece that was promoted, if any. If this is type NoPiece then the
         *  move did not involve a promotion. */
        Piece PiecePromoted;

        MoveData();
    };

    explicit GameLogic(QObject * = 0);
    virtual ~GameLogic();

    /** Returns the game board. */
    Board const &GetBoard() const{ return m_board; }


    /** Sets up the board for a new game. */
    virtual void SetupNewGame(SetupTypeEnum = StandardChess);

    /** Convenience function clears the board. */
    void Clear(){ SetupNewGame(Empty); }

    /** Returns a list of squares occupied by the type of piece specified.
     *  The list will be empty if it didn't find any.
     *
     *  This lookup is done in O(log(N)) time, where N is the number of different types of pieces.
    */
    GUtil::DataObjects::Vector<ISquare const *> FindPieces(Piece::AllegienceEnum,
                                                          Piece::PieceTypeEnum) const;

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
    virtual MoveValidationEnum ValidateMove(const ISquare &source, const ISquare &destination) const;


    /** Returns a list of squares that are valid for the given square and piece. */
    ::GUtil::DataObjects::Vector<ISquare const *> GetPossibleMoves(const ISquare &, const Piece &) const;


    /** Moves based on a PGN_MoveData object, which is created from the PGN parser. */
    void Move(const PGN_MoveData &);

    /** Moves based on a MoveData object. */
    void Move(const MoveData &);

    /** Causes the last move to be undone. */
    void Undo();

    /** Causes the last move that was undone to be redone. */
    void Redo();

    /** Returns the allegience whose turn it is. */
    Piece::AllegienceEnum WhoseTurn() const;


signals:

    /** This is emitted whenever a piece is moved. */
    void NotifyMove(const GameLogic::MoveData &);

    /** This is emitted whenever a new game is set up.
     *  The optional argument is the setup type, which usually is StandardChess, but
     *  it will also be Empty if the board was cleared.
    */
    void NotifyNewGame(int setup_type = StandardChess);


private:

    // This facilitates fast piece lookups
    GUtil::DataObjects::Map<Piece::PieceTypeEnum, ISquare const *> m_whitePieceIndex;
    GUtil::DataObjects::Map<Piece::PieceTypeEnum, ISquare const *> m_blackPieceIndex;

    GUtil::DataObjects::Vector<MoveData> m_moveHistory;
    GINT32 m_moveHistoryIndex;

    /** Maps a MoveData object to our own move_data type. */
    MoveData _translate_move_data(const PGN_MoveData &);

    void _move(const MoveData &, bool reverse = false);
    void _init_piece(int, int, Piece::AllegienceEnum, Piece::PieceTypeEnum);

};


}

#endif // GKCHESS_GAMELOGIC_H
