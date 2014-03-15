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

#ifndef GKCHESS_ISQUARE_H
#define GKCHESS_ISQUARE_H

#include "gkchess_globals.h"

NAMESPACE_GKCHESS;


class Piece;


/** Describes one square of the chess board.
 *  This is an interface because its implementation depends on
 *  the board's implementation
*/
class ISquare
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

    /** Returns the piece on the square. If there is no piece, then a null pointer is returned.
    */
    virtual Piece const *GetPiece() const = 0;

    /** \} */

    

    /** \name Convenience Functions
     *  Here are functions for your convenience
     *  \{
    */


    /** A convenience function that tells you if this square is light or dark. */
    bool IsDarkSquare() const{ 
        return (0x1 & GetRow()) == (0x1 & GetColumn()); 
    }

    /** Compares squares based on their row and column. */
    bool operator == (const ISquare &other){
        return GetColumn() == other.GetColumn() &&
                GetRow() == other.GetRow();
    }
    
    /** Compares squares based on their row and column. */
    bool operator != (const ISquare &other){
        return GetColumn() != other.GetColumn() ||
                GetRow() != other.GetRow();
    }


    /** \} */

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_ISQUARE_H
