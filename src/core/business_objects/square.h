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

#ifndef GKCHESS_SQUARE_H
#define GKCHESS_SQUARE_H

#include "gkchess_piece.h"

NAMESPACE_GKCHESS;


/** Describes one square of the chess board.
 *  This is an abstract interface, because its implementation depends on
 *  the board's implementation
*/
class Square
{
public:

    /** \name Interface
     *  This is the interface you have to implement
     *  \{
    */

    /** Holds the column that the square is in. */
    virtual int GetColumn() const = 0;

    /** Holds the row that the square is in. */
    virtual int GetRow() const = 0;

    /** Returns the piece on the square. If there is no piece, then the return
     *  object will have piece type NoPiece.
    */
    virtual Piece GetPiece() const = 0;

    /** Returns true if en passant is available on the square. */
    virtual bool GetEnPassantAvailable() const = 0;

    /** \} */

    virtual ~Square();


    /** \name Convenience Functions
     *  Here are functions for your convenience
     *  \{
    */


    /** A convenience function that tells you if this square is light or dark. */
    bool IsDarkSquare() const;

    bool operator == (const Square &);
    bool operator != (const Square &);


    /** \} */

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_SQUARE_H
