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
#include "gutil_map.h"
#include <QObject>

namespace GKChess{

class Piece;


/** The implementation for a variable sized chess board. You can adjust the number of columns
 *  but not the number of rows.
 *
 * \sa AbstractBoard
*/
class Board :
        public AbstractBoard
{
    const int m_columnCount;
    Square *m_squares;
    GUtil::Map<Piece, SquarePointerConst> m_index;
public:

    /** Constructs a board with the given number of columns.
     *  It defaults to the standard chess board with 8 columns.
    */
    Board(int num_cols = 8);

    Board(const Board &o);
    Board(const AbstractBoard &o);
    Board &operator = (const AbstractBoard &o);

    virtual ~Board();


    /** \name AbstractBoard interface
     *  \{
    */
    int ColumnCount() const;
    int RowCount() const;
    Square const &SquareAt(int col, int row) const;
    virtual void SetPiece(Piece const &p, const Square &s);
    GUtil::Vector<Square const *> FindPieces(Piece const &pc) const;

    /** \}*/


private:

    Square &square_at(int, int);

    void _init();
    void _copy_construct(const AbstractBoard &o);
    void _copy_board(const AbstractBoard &o);
    void _init_index();

};



/** A chess board that is observable to views. */
class ObservableBoard :
        public QObject,
        public Board
{
    Q_OBJECT
public:

    ObservableBoard(int num_cols = 8, QObject * = 0);
    ObservableBoard(const AbstractBoard &);
    ObservableBoard &operator = (const AbstractBoard &);
    virtual ~ObservableBoard();

    /** Emits the proper signals when a piece is changed. */
    void SetPiece(Piece const &p, Square const &s);


signals:

    /** This signal is emitted to notify whenever a square is about to be updated with SetPiece(). */
    void NotifySquareAboutToBeUpdated(const GKChess::Square &);

    /** This signal is emitted to notify whenever a square has been updated with SetPiece(). */
    void NotifySquareUpdated(const GKChess::Square &);


    /** This signal is emitted whenever a piece is about to be moved with the Move() function. */
    void NotifyPieceAboutToBeMoved(const GKChess::MoveData &);

    /** This signal is emitted whenever a piece is moved with the Move() function. */
    void NotifyPieceMoved(const GKChess::MoveData &);

    /** This signal is emitted after a side resigns. */
    void NotifyResignation(Piece::AllegienceEnum);


protected:

    /** Emits the proper signals when a piece is moved. */
    void move_p(const MoveData &);

};


}

#endif // GKCHESS_BOARD_H
