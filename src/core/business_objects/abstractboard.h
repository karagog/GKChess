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

#ifndef GKCHESS_ABSTRACTBOARD_H
#define GKCHESS_ABSTRACTBOARD_H

#include "gkchess_piece.h"
#include <QObject>

namespace GKChess{

class ISquare;
class PGN_MoveData;


/** Describes a chess board interface. */
class AbstractBoard :
        public QObject
{
    Q_OBJECT
    GUTIL_DISABLE_COPY(AbstractBoard);    
public:

    /** The board class has no concept of what these variables mean, it just stores the data. */
    class IGameState
    {
    public:
        /** Whose turn it is. */
        virtual Piece::AllegienceEnum GetWhoseTurn() const = 0;
        virtual void SetWhoseTurn(Piece::AllegienceEnum) = 0;

        /** A castle column, 0 based. If this castling move has been executed, or otherwise
         *  the opportunity ruined, it will be -1
        */
        virtual int GetCastleWhite1() const = 0;
        virtual void SetCastleWhite1(int) = 0;
        /** A castle column, 0 based. If this castling move has been executed, or otherwise
         *  the opportunity ruined, it will be -1
        */
        virtual int GetCastleWhite2() const = 0;
        virtual void SetCastleWhite2(int) = 0;
        /** A castle column, 0 based. If this castling move has been executed, or otherwise
         *  the opportunity ruined, it will be -1
        */
        virtual int GetCastleBlack1() const = 0;
        virtual void SetCastleBlack1(int) = 0;
        /** A castle column, 0 based. If this castling move has been executed, or otherwise
         *  the opportunity ruined, it will be -1
        */
        virtual int GetCastleBlack2() const = 0;
        virtual void SetCastleBlack2(int) = 0;

        /** The en passant square, if there is one. If not then this is null. */
        virtual ISquare const *GetEnPassantSquare() const = 0;
        virtual void SetEnPassantSquare(ISquare const *) = 0;

        /** The current number of half-moves since the last capture or pawn advance.
         *  This is used for determining a draw from lack of progress.
        */
        virtual int GetHalfMoveClock() const = 0;
        virtual void SetHalfMoveClock(int) = 0;

        /** Returns the current full move number. */
        virtual int GetFullMoveNumber() const = 0;
        virtual void SetFullMoveNumber(int) = 0;

        virtual ~IGameState(){}

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



    AbstractBoard(QObject * = 0);
    virtual ~AbstractBoard();

    /** Populates this board with the position given in X-FEN notation.

        This has a default implementation that should work for all board
        implementations, but it is left virtual in case you want to optimize it
        for your board implementation.
    */
    virtual void FromFEN(const GUtil::String &);

    /** Serializes the board object into a FEN string.
        \note The default implementation should work for all board implementations,
        but it is left virtual in case you want to customize/optimize it.
    */
    virtual GUtil::String ToFEN() const;

    /** Sets a piece on the square. If the square was occupied then
     *  it will simply be replaced by the new one. If you pass Piece::NoPiece
     *  then the space will be cleared.
     *
     *  The signal NotifySquareUpdated will be emitted.
     *
     *  \warning It is not the responsibility of this class to check inputs for valid bounds
    */
    void SetPiece(const Piece &, ISquare const &);

    /** Convenience function returns the piece on the given square. */
    Piece const *GetPiece(int column, int row) const;

    /** Moves based on a MoveData object. You can create one via GenerateMoveData. */
    void Move(const MoveData &);

    /** Returns a reference to the square at the given column and row.
     *  The square is valid as long as the board is, so you can safely pass around pointers to it.
     * \warning It is not the responsibility of this class to check inputs for valid bounds
    */
    virtual ISquare const &SquareAt(int column, int row) const = 0;

    /** Returns the number of rows. */
    virtual int RowCount() const = 0;

    /** Returns the number of columns. */
    virtual int ColumnCount() const = 0;

    /** Returns the current game state, as a reference for best performance. */
    virtual IGameState const &GameState() const = 0;

    /** Returns the current game state, as a reference for best performance. You
     *  modify it directly.
    */
    virtual IGameState &GameState() = 0;

    /** Returns a list of squares occupied by the type of piece specified.
     *  The list will be empty if there are no such pieces on the board.
     *
     *  This lookup should be done at least as good as O(log(N)) time, where N is the
     *  number of different types of pieces.
    */
    virtual GUtil::Vector<ISquare const *> FindPieces(const Piece &) const = 0;

    /** This function has a basic implementation provided for convenience, which merely
     *  iterates through the squares and removes their pieces. Override it if you like.
    */
    virtual void Clear();


signals:

    /** This signal is emitted to notify whenever a square is about to be updated with SetPiece(). */
    void NotifySquareAboutToBeUpdated(const GKChess::ISquare &);

    /** This signal is emitted to notify whenever a square has been updated with SetPiece(). */
    void NotifySquareUpdated(const GKChess::ISquare &);


    /** This signal is emitted whenever a piece is about to be moved with the Move() function. */
    void NotifyPieceAboutToBeMoved(const GKChess::AbstractBoard::MoveData &);

    /** This signal is emitted whenever a piece is moved with the Move() function. */
    void NotifyPieceMoved(const GKChess::AbstractBoard::MoveData &);


protected:

    /** You must implement moving pieces, but don't emit any signals. */
    virtual void move_p(const MoveData &) = 0;

    /** You must implement setting pieces on the board, but don't emit any signals. */
    virtual void set_piece_p(const Piece &, int col, int row) = 0;

};


}

#endif // GKCHESS_ABSTRACTBOARD_H
