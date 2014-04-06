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

#ifndef GKCHESS_IGAMELOGIC_H
#define GKCHESS_IGAMELOGIC_H

#include "gkchess_imovevalidator.h"
#include "gkchess_piece.h"

NAMESPACE_GKCHESS;

class PGN_MoveData;


/** Defines the game logic interface.
 *  This is the set of functions required to play chess with this library.
*/
class IGameLogic :
        public IMoveValidator
{
public:

    /** An interface that encapsulates feedback from the user, such as choosing a promoted piece.
     *  The implementation should leverage the user interface to prompt the user for the given input.
    */
    class IUserFeedback
    {
    public:

        /** This will be called whenever the game logic needs input from
         *  the user to decide what piece they want to promote to.
         *
         *  \note This is only relevant when generating move data by two squares. If you are
         *  generating moves from PGN then it's already in the notation (i.e. e8=Q)
        */
        virtual Piece ChoosePromotedPiece() = 0;

    };


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


    /** Holds all the information we need to do a move. */
    struct MoveData
    {
        /** The half-move number for the move. */
        int PlyNumber;

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

        /** The position of the board before the move, in FEN notation. */
        GUtil::String CurrentPosition_FEN;

        /** Returns true if this is a null move data (default constructed). */
        bool IsNull() const{ return -1 == PlyNumber; }

        MoveData()
            :PlyNumber(-1),
              Source(0),
              Destination(0),
              CastleType(NoCastle)
        {}

    };


    /** Encodes the different possible results of a move validation. */
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

        /** Invalid because there was a problem with the parameters you gave the function.
         *  For example if you didn't specify a source and destination square.
        */
        InvalidInputError = 4,


        /** If you are extending this class for your own custom rules then you'll base
         *  your validation types with this value.
        */
        CustomInvalidMoveOffset = 100
    };


    /** Returns the game board. */
    virtual AbstractBoard const &GetBoard() const = 0;

    /** Sets up the board for a new game. */
    virtual void SetupNewGame(SetupTypeEnum = StandardChess) = 0;

    /** Moves based on a MoveData object. You can create one via GenerateMoveData. */
    virtual void Move(const MoveData &) = 0;

    /** Creates a MoveData object from a source and dest square input.
     *  You must supply a user feedback object, so the game logic knows how the
     *  user wants to proceed in the event of a pawn promotion.
     *
     *  \note This function also works with invalid moves, so if you care about
     *  validation you have to call ValidateMove before generating the move data.
    */
    virtual MoveData GenerateMoveData(const ISquare *source,
                                      const ISquare *dest,
                                      IUserFeedback *) const = 0;

    /** Creates a MoveData object from a PGN MoveData object. */
    virtual MoveData GenerateMoveData(const PGN_MoveData &) const = 0;

    /** Validates the move. */
    virtual MoveValidationEnum ValidateMove(const ISquare &source,
                                            const ISquare &dest) const = 0;


    /** Returns a list of valid squares that the piece on the given square can move to. */
    virtual GUtil::Vector<ISquare const *> GetValidMovesForSquare(const ISquare &) const = 0;


    /** Convenience function clears the board. */
    void Clear(){ SetupNewGame(Empty); }

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_IGAMELOGIC_H
