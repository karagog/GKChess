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


/** The implementation for a variable sized chess board. In general you should use
 *  one of the derivatives of Board.
 *
 *  See AbstractBoard for the interface description.
 * \sa AbstractBoard
*/
template<GUINT32 NUM_COLS>

class Board_Imp :
        public AbstractBoard
{
    Square *m_squares;
    GUtil::Map<Piece, SquarePointerConst> m_index;

public:

    Board_Imp(){ _init(); }

    Board_Imp(const Board_Imp<NUM_COLS> &o){
        _copy_construct(o);
    }

    Board_Imp(const AbstractBoard &o){
        _copy_construct(o);
    }

    Board_Imp &operator = (const AbstractBoard &o){
        if(ColumnCount() != o.ColumnCount() || RowCount() != o.RowCount())
            THROW_NEW_GUTIL_EXCEPTION2(GUtil::Exception, "Board size mismatch");
        _copy_board(o);
        m_index.Clear();
        _init_index();
    }

    virtual ~Board_Imp(){
        free(m_squares);
    }


    /** \name AbstractBoard interface
     *  \{
    */
    int ColumnCount() const{ return NUM_COLS; }

    int RowCount() const{ return 8; }

    Square const &SquareAt(int col, int row) const{ return m_squares[__map_2d_indices_to_1d(col, row)]; }

    virtual void SetPiece(Piece const &p, const Square &s){
        Piece orig = s.GetPiece();
        AbstractBoard::SetPiece(p, s);

        // Remove the old piece from the index
        if(!orig.IsNull())
            m_index.Remove(orig, &s);
        // Add the new piece to the index
        if(!p.IsNull())
            m_index.InsertMulti(p, &s);
    }

    GUtil::Vector<Square const *> FindPieces(Piece const &pc) const
    {
        GUtil::Vector<Square const *> ret;
        if(Piece::NoPiece == pc.GetType()){
            if(Piece::AnyAllegience == pc.GetAllegience()){
                // Append all pieces to the list
                for(typename GUtil::Map<Piece, SquarePointerConst>::const_iterator iter = m_index.begin();
                    iter != m_index.end(); ++iter){
                    G_FOREACH_CONST(Square const *sqr, iter->Values())
                        ret.PushBack(sqr);
                }
            }
            else{
                // Append only pieces of the given allegience to the list
                for(typename GUtil::Map<Piece, Square const *>::const_iterator iter = m_index.begin();
                    iter != m_index.end(); ++iter){
                    if(iter->Key().GetAllegience() == pc.GetAllegience()){
                        G_FOREACH_CONST(Square const *sqr, iter->Values())
                            ret.PushBack(sqr);
                    }
                }
            }
        }
        else{
            ret = m_index.Values(pc);
        }
        return ret;
    }

    /** \}*/


protected:

    /** \name AbstractBoard interface
     *  \{
    */

    virtual Square &square_at(int c, int r){
        return m_squares[__map_2d_indices_to_1d(c, r)];
    }

    /** \} */


private:

    void _init(){
        // Initialize all the squares in the array
        m_squares = (Square *)malloc(ColumnCount() * RowCount() * sizeof(Square));
        for(int c = 0; c < ColumnCount(); ++c)
            for(int r = 0; r < RowCount(); ++r)
                new(&square_at(c, r)) Square(c, r);
    }

    void _copy_construct(const AbstractBoard &o){
        if(ColumnCount() != o.ColumnCount() || RowCount() != o.RowCount())
            THROW_NEW_GUTIL_EXCEPTION2(GUtil::Exception, "Board size mismatch");
        _init();
        _copy_board(o);
        _init_index();
    }

    void _copy_board(const AbstractBoard &o){
        // copy the contents of the other board's squares
        for(int c = 0; c < ColumnCount(); ++c){
            for(int r = 0; r < RowCount(); ++r){
                square_at(c, r) = o.SquareAt(c, r);
            }
        }
    }

    void _init_index(){
        for(int c = 0; c < ColumnCount(); ++c){
            for(int r = 0; r < RowCount(); ++r){
                const Square &cur = SquareAt(c, r);
                if(cur.GetPiece())
                    m_index.InsertMulti(cur.GetPiece(), &cur);
            }
        }
    }


    /** Maps col-row indices to a 1-dimensional index. */
    static int __map_2d_indices_to_1d(GUINT32 col, GUINT32 row)
    {
        return (col << 3) | row;
    }

};



/** A standard chess board with 8 rows and columns. */
class Board :
        public Board_Imp<8>
{
public:
    Board(){}
    Board(const AbstractBoard &o) :Board_Imp<8>(o) {}
    Board &operator = (const AbstractBoard &o){ Board_Imp<8>::operator = (o); return *this;}
    virtual ~Board(){}
};



/** The interface that must be implemented to be usable with the boardview. */
class IObservableBoard :
        public QObject
{
    Q_OBJECT
public:
    IObservableBoard(QObject *p = 0) :QObject(p) {}

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

};



/** A standard chess board that is observable to views. */
class ObservableBoard :
        public IObservableBoard,
        public Board
{
    Q_OBJECT
public:

    ObservableBoard(QObject *p = 0):IObservableBoard(p){}
    ObservableBoard(const AbstractBoard &o) :IObservableBoard(0), Board(o) {}
    ObservableBoard &operator = (const AbstractBoard &o){ Board::operator = (o); return *this;}
    virtual ~ObservableBoard(){}

    void SetPiece(Piece const &p, Square const &s);

protected:

    void move_p(const MoveData &);

};


}

#endif // GKCHESS_BOARD_H
