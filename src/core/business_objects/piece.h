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
class Piece :
        public GUtil::Utils::SharedData
{
    friend class Board;
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
        Pawn = 5
    };

    /** Describes which side the piece fights on.
     *  \note The enumeration values correspond to the unicode point offset
    */
    enum AllegienceEnum
    {
        White = 0x2654,
        Black = 0x265A,
    };


    /** Constructs a piece with the given allegience and type. */
    explicit Piece(AllegienceEnum, PieceTypeEnum);
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
    inline int UnicodeValue() const{ return (int)GetAllegience() + GetType(); }

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_PIECE_H
