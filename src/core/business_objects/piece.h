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

NAMESPACE_GKCHESS;


/** Describes a chess piece. */
class Piece
{
public:

    /** Describes all the different types of pieces. */
    enum PieceTypeEnum
    {
        InvalidPiece = 0,

        Pawn = 1,
        Knight = 2,
        Bishop = 3,
        Rook = 4,
        Queen = 5,
        King = 6,

        /** If you are extending the library you can use this offset as a base
         *  for your own custom pieces.
        */
        CustomPieceOffset = 10
    };

    /** Describes which side the piece fights on. */
    enum AllegienceEnum
    {
        InvalidAllegience = 0,

        White = 1,
        Black = 2,

        /** If you are extending the library you can use this offset as a base
         *  for your own custom allegiences.
        */
        CustomAllegienceOffset = 10
    };


    /** Constructs a piece with the given allegience and type. */
    explicit Piece(AllegienceEnum, PieceTypeEnum);
    virtual ~Piece();

    PROPERTY(Type, PieceTypeEnum);
    PROPERTY(Allegience, AllegienceEnum);

    /** Returns a human-readable string to describe the piece. */
    virtual ::GUtil::DataObjects::String ToString(bool with_allegience = false) const;

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_PIECE_H
