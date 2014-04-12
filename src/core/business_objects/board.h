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

class Piece;


/** The implementation for a standard chess board.
 *  See AbstractBoard for the interface description.
 * \sa AbstractBoard
*/
class Board :
        public AbstractBoard
{
    Q_OBJECT

    // Very secret...
    void *g_d;
public:

    Board(QObject * = 0);
    Board(const AbstractBoard &);
    Board &operator = (const AbstractBoard &);
    virtual ~Board();


    /** \name AbstractBoard interface
     *  \{
    */
    virtual int ColumnCount() const;
    virtual int RowCount() const;
    virtual ISquare const &SquareAt(int column, int row) const;
    virtual GUtil::Vector<ISquare const *> FindPieces(const Piece &) const;

    virtual Piece::AllegienceEnum GetWhoseTurn() const;
    virtual void SetWhoseTurn(Piece::AllegienceEnum v);
    virtual int GetCastleWhite1() const;
    virtual void SetCastleWhite1(int v);
    virtual int GetCastleWhite2() const;
    virtual void SetCastleWhite2(int v);
    virtual int GetCastleBlack1() const;
    virtual void SetCastleBlack1(int v);
    virtual int GetCastleBlack2() const;
    virtual void SetCastleBlack2(int v);
    virtual ISquare const *GetEnPassantSquare() const;
    virtual void SetEnPassantSquare(ISquare const *v);
    virtual int GetHalfMoveClock() const;
    virtual void SetHalfMoveClock(int v);
    virtual int GetFullMoveNumber() const;
    virtual void SetFullMoveNumber(int v);
    virtual ResultTypeEnum GetResult() const;
    virtual void SetResult(ResultTypeEnum r);
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

};


}

#endif // GKCHESS_BOARD_H
