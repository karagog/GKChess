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

#ifndef GKCHESS_PIECE_H
#define GKCHESS_PIECE_H

#include "gkchess_globals.h"
#include "gutil_strings.h"
#include "gutil_smartpointer.h"

NAMESPACE_GKCHESS;


/** Describes a chess piece. */
class Piece
{
public:

    /** Describes all the different types of pieces.
     *  \note The enumeration values correspond to the unicode point offset
    */
    enum PieceTypeEnum
    {
        King = 0,
        Queen = 1,
        Rook = 2,
        Bishop = 3,
        Knight = 4,
        Pawn = 5,

        NoPiece = 7
    };

    /** Describes which side the piece fights on.
     *  \note The enumeration values correspond to the unicode point offset
    */
    enum AllegienceEnum
    {
        AnyAllegience = 0,

        White = 0x2654,
        Black = 0x265A,
    };


    /** Constructs an empty piece (type NoPiece). */
    Piece();
    
    /** Constructs a piece with the given allegience and type. */
    Piece(PieceTypeEnum, AllegienceEnum = AnyAllegience);
    
    virtual ~Piece();

    /** Returns the type of the piece. */
    READONLY_PROPERTY(Type, PieceTypeEnum);

    /** Returns the allegience of the piece. */
    READONLY_PROPERTY(Allegience, AllegienceEnum);

    /** Returns a human-readable string to describe the piece. */
    virtual ::GUtil::DataObjects::String ToString(bool with_allegience = false) const;

    /** Returns the unicode point of the piece. You can display this in any text viewer
     *  that supports unicode characters.
    */
    int UnicodeValue() const;
    
    /** Returns the UTF-8 multi-byte representation of the piece. */
    GUtil::DataObjects::String GetUtf8Char() const;
    
    /** Returns the ascii representation of the piece. */
    char GetAsciiChar() const;

    /** This translates a unicode point to a chess piece. The code point
     *  can either be the ascii characters or the unicode chess pieces.
    */
    static PieceTypeEnum GetTypeFromUnicodeValue(int);

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_PIECE_H
