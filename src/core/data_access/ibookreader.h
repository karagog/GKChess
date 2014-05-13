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

#ifndef GKCHESS_IBOOKREADER_H
#define GKCHESS_IBOOKREADER_H

#include "gutil_strings.h"
#include <QObject>

namespace GKChess
{


/** An interface to access an opening book. */
class IBookReader
{
public:

    /** Describes a move. The row and col indices are both starting at 0. */
    struct Move
    {
        /** The text that describes the move.  i.e. e2e4  e1h1 */
        GUtil::String Text;

        /** The weight is a percentage between 0 and 100.
         *  All moves in the list from LookupMoves() will sum to have a total weight of 100.
        */
        float Weight;
    };

    /** Opens a book with the given file name. If a book was already open, it will be closed
     *  and the new one will be opened.
     *
     *  Throws an exception if the book could not be opened.
    */
    virtual void OpenBook(const char *) = 0;

    /** Returns the filename for the book that is currently open, or 0 if there is no book open. */
    virtual const char *GetBookFilename() const = 0;

    /** Looks up the given position in the book, and returns all the moves it found.
     *  If the position was not in the database, an empty vector is returned.
    */
    virtual GUtil::Vector<Move> LookupMoves(const char *fen) = 0;

    /** Closes the book, or does nothing if it's already closed. */
    virtual void CloseBook() = 0;

    virtual ~IBookReader(){}

};


}

Q_DECLARE_INTERFACE(GKChess::IBookReader, "GKChess.IBookReader")

#endif // GKCHESS_IBOOKREADER_H