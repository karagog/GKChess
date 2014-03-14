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


/** The implementation for the GKChess board.
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
        // For fast lookups
        Piece m_piece;
        char m_column;
        char m_row;
        bool m_enPassantAvailable;
    public:

        Square(int col, int row);
        virtual int GetColumn() const;
        virtual int GetRow() const;
        virtual bool GetEnPassantAvailable() const;
        virtual Piece const *GetPiece() const;

        /** This function is not part of the interface, but it's necessary anyways */
        void SetPiece(const Piece &p);
    };
    
    GUtil::DataObjects::Vector<Square> m_squares;
public:

    explicit Board(QObject *parent = 0);
    virtual ~Board();


    /** \name AbstractBoard interface
     *  \{
    */
    virtual void SetPiece(const Piece &, int column, int row);
    virtual ISquare const &GetSquare(int column, int row) const;
    /** \}*/

};


}

#endif // GKCHESS_BOARD_H
