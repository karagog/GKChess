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
    
    GUtil::Vector<Square> m_squares;
    Piece::AllegienceEnum m_currentTurn;
    int m_halfMoveClock;
    int m_fullMoveNumber;
    ISquare const *m_enPassantSquare;
    GUINT8 m_whiteCastleInfo;
    GUINT8 m_blackCastleInfo;
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
    virtual void SetPiece(const Piece &, int column, int row);
    virtual ISquare const &SquareAt(int column, int row) const;
    virtual Piece::AllegienceEnum GetWhoseTurn() const;
    virtual void SetWhoseTurn(Piece::AllegienceEnum);
    virtual int GetHalfMoveClock() const;
    virtual void SetHalfMoveClock(int);
    virtual int GetFullMoveNumber() const;
    virtual void SetFullMoveNumber(int);
    virtual ISquare const *GetEnPassantSquare() const;
    virtual void SetEnPassantSquare(ISquare const *);
    virtual GUINT8 GetCastleInfo(Piece::AllegienceEnum) const;
    virtual void SetCastleInfo(Piece::AllegienceEnum, GUINT8);
    /** \}*/

};


}

#endif // GKCHESS_BOARD_H
