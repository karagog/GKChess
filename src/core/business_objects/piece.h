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
#include <QMetaType>

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

        /** A special piece from Capablanca chess. */
        Archbishop = 6,

        /** A special piece from Capablanca chess. */
        Chancellor = 7,

        NoPiece = -1
    };

    /** Describes which side the piece fights on. */
    enum AllegienceEnum
    {
        AnyAllegience = -1,

        White = 0,
        Black = 1,
    };


    /** Constructs an empty piece (type NoPiece). */
    Piece();

    /** Constructs a piece with the given allegience and type. */
    explicit Piece(PieceTypeEnum, AllegienceEnum = AnyAllegience);

    bool operator == (Piece const &o) const{
        return GetAllegience() == o.GetAllegience() &&
                GetType() == o.GetType();
    }
    bool operator != (Piece const &o) const{
        return GetAllegience() != o.GetAllegience() ||
                GetType() != o.GetType();
    }

    /** A less-than operator so you can index by the piece type. */
    bool operator < (Piece const &o) const{
        return 100 * GetAllegience() + GetType() <
               100 * o.GetAllegience() + o.GetType();
    }

    /** Returns the type of the piece. */
    READONLY_PROPERTY(Type, PieceTypeEnum);

    /** Returns the allegience of the piece. */
    READONLY_PROPERTY(Allegience, AllegienceEnum);

    /** Returns the opposite allegience to this piece.
     *  The behavior is undefined if this does not have an allegience.
    */
    AllegienceEnum GetOppositeAllegience() const{ return (AllegienceEnum)(~(int)GetAllegience() & 0x1); }

    /** Returns true if this piece has no type. */
    bool IsNull() const{ return NoPiece == GetType(); }

    /** Returns a human-readable string to describe the piece. */
    GUtil::String ToString(bool with_allegience = false) const;

    /** Returns a human-readable string for the piece type. */
    static GUtil::String TypeToString(PieceTypeEnum);

    /** Returns the unicode point of the piece. You can display this in any text viewer
     *  that supports unicode characters.
    */
    int UnicodeValue() const;

    /** Returns the UTF-8 multi-byte representation of the piece. */
    GUtil::String GetUtf8Char() const;

    /** Returns the PGN representation of the piece, which does not have allegience information,
     *  and pawns are an empty string. */
    char const *ToPGN() const;

    /** Returns the PGN representation of the piece, which does not have allegience information,
     *  and pawns are an empty string. */
    static char const *ToPGN(PieceTypeEnum);

    /** Returns the piece type from the PGN string. */
    static PieceTypeEnum GetTypeFromPGN(char);

    /** Returns the ascii FEN representation of the piece, which encodes allegience information
     *  in the case of the char, so white is in capital letters and black is lower case.
    */
    char ToFEN() const;

    /** Returns a piece initialized with a FEN notation character. */
    static Piece FromFEN(char);

    /** This translates a unicode point to a chess piece. The code point
     *  can either be the ascii characters or the unicode chess pieces.
    */
    static PieceTypeEnum GetTypeFromUnicodeValue(int);

};


END_NAMESPACE_GKCHESS;


// This is so we can use this class with QVariant
Q_DECLARE_METATYPE(GKChess::Piece)


#endif // GKCHESS_PIECE_H
