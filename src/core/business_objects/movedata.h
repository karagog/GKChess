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

#ifndef GKCHESS_MOVEDATA_ENGINE_H
#define GKCHESS_MOVEDATA_ENGINE_H

#include "gkchess_globals.h"

NAMESPACE_GKCHESS;


/** Represents a move from any source, which just has the most basic data: source and dest
 *  squares and the promoted piece.
*/
struct GenericMove
{
    GUINT8 SourceCol, SourceRow;
    GUINT8 DestCol, DestRow;

    /** A character describing which piece to promote.  If there is no promoted piece it will be 0. */
    char PromotedPiece;

    GenericMove()
        :SourceCol(0), SourceRow(0),
          DestCol(0), DestRow(0),
          PromotedPiece(0)
    {}
    GenericMove(GUINT8 source_col, GUINT8 source_row,
                      GUINT8 dest_col, GUINT8 dest_row,
                      char promoted_piece = 0)
        :SourceCol(source_col), SourceRow(source_row),
          DestCol(dest_col), DestRow(dest_row),
          PromotedPiece(promoted_piece)
    {}
};


/** Represents a move that came from an opening book.  Each move has a weight and learn value. */
struct BookMove :
        public GenericMove
{
    /** The weight is a percentage between 0 and 100.
     *  All moves in the list from LookupMoves() will sum to have a total weight of 100.
    */
    float Weight;

    /** The 32-bit application-dependent "learn" value.  If you don't know what this is you can
     *  safely ignore it.
    */
    GUINT32 Learn;

    BookMove()
        :Weight(0), Learn(0) {}

    BookMove(float weight, GUINT32 learn,
             GUINT8 source_col, GUINT8 source_row,
             GUINT8 dest_col, GUINT8 dest_row,
             char promoted_piece = 0)
        :GenericMove(source_col,source_row, dest_col, dest_row, promoted_piece),
          Weight(weight), Learn(learn)
    {}
};


/** A class that represents the move data returned by an engine. */
struct EngineMove :
        public GenericMove
{
    /** The move's score in centipawns. */
    int Score;
};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_MOVEDATA_ENGINE_H
