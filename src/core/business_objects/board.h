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
#include "gkchess_piece.h"
#include "gkchess_isquare.h"
#include "gutil_map.h"

namespace GKChess{


/** The implementation for a standard chess board.
 *  See AbstractBoard for the interface description.
 * \sa AbstractBoard
*/
class Board :
        public AbstractBoard
{
    Q_OBJECT
    
    /** Our square implementation. */
    class Square :
            public ISquare
    {
        // For fast lookups this is implemented as quickly queryable data types
        Piece m_piece;
        int m_column;
        int m_row;
    public:

        Square(int col, int row);
        virtual int GetColumn() const;
        virtual int GetRow() const;
        virtual Piece const *GetPiece() const;

        /** This function is not part of the interface, but it's necessary anyways */
        void SetPiece(const Piece &p);
    };

    class GameState : public IGameState
    {
        Piece::AllegienceEnum WhoseTurn;
        int CastleWhite1;
        int CastleWhite2;
        int CastleBlack1;
        int CastleBlack2;
        ISquare const *EnPassantSquare;
        int HalfMoveClock;
        int FullMoveNumber;
    public:
        Piece::AllegienceEnum GetWhoseTurn() const{ return WhoseTurn; }
        void SetWhoseTurn(Piece::AllegienceEnum v){ WhoseTurn=v; }
        int GetCastleWhite1() const{ return CastleWhite1; }
        void SetCastleWhite1(int v){ CastleWhite1=v; }
        int GetCastleWhite2() const{ return CastleWhite2; }
        void SetCastleWhite2(int v){ CastleWhite2=v; }
        int GetCastleBlack1() const{ return CastleBlack1; }
        void SetCastleBlack1(int v){ CastleBlack1=v; }
        int GetCastleBlack2() const{ return CastleBlack2; }
        void SetCastleBlack2(int v){ CastleBlack2=v; }
        ISquare const *GetEnPassantSquare() const{ return EnPassantSquare; }
        void SetEnPassantSquare(ISquare const *v){ EnPassantSquare=v; }
        int GetHalfMoveClock() const{ return HalfMoveClock; }
        void SetHalfMoveClock(int v){ HalfMoveClock=v; }
        int GetFullMoveNumber() const{ return FullMoveNumber; }
        void SetFullMoveNumber(int v){ FullMoveNumber=v; }
        GameState();
        GameState(const IGameState &);
    };
    
    GUtil::Vector<Square> m_squares;
    GameState m_gameState;

    // These facilitate fast piece lookups
    GUtil::Map<Piece::PieceTypeEnum, ISquare const *> m_whitePieceIndex;
    GUtil::Map<Piece::PieceTypeEnum, ISquare const *> m_blackPieceIndex;

public:

    explicit Board(QObject *parent = 0);
    Board(const AbstractBoard &);
    Board &operator = (const AbstractBoard &);
    virtual ~Board();


    /** \name AbstractBoard interface
     *  \{
    */
    virtual int ColumnCount() const;
    virtual int RowCount() const;
    virtual ISquare const &SquareAt(int column, int row) const;
    virtual IGameState const &GameState() const{ return m_gameState; }
    virtual IGameState &GameState(){ return m_gameState; }
    virtual GUtil::Vector<ISquare const *> FindPieces(const Piece &) const;
    /** \}*/


protected:

    /** \name AbstractBoard interface
     *  \{
    */
    virtual void set_piece_p(const Piece &, int, int);
    virtual void move_p(const MoveData &);
    /** \} */


private:

    void _init();
    Square &_square_at(int col, int row);
    GUtil::Map<Piece::PieceTypeEnum, ISquare const *> &_index(Piece::AllegienceEnum);
    GUtil::Map<Piece::PieceTypeEnum, ISquare const *> const &_index(Piece::AllegienceEnum) const;

};


}

#endif // GKCHESS_BOARD_H
