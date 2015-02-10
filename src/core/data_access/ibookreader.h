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

#include "gkchess_movedata.h"
#include <gutil/string.h>
#include <QObject>

namespace GKChess
{


/** An interface to access an opening book. */
class IBookReader
{
public:

    class IValidationProgressObserver;


    /** Opens a book with the given file name. If a book was already open, it will be closed
     *  and the new one will be opened.
     *
     *  Throws an exception if the book could not be opened.
    */
    virtual void OpenBook(const char *) = 0;

    /** Returns true if the book is open. */
    virtual bool IsBookOpen() const = 0;

    /** Returns the filename for the book that is currently open, or 0 if there is no book open. */
    virtual const char *GetBookFilename() const = 0;

    /** Validates the book. This may be a time-expensive operation, so you can provide a callback function
     *  to track its progress.
     *  Throws an exception if validation fails
    */
    virtual void ValidateBook(IValidationProgressObserver * = 0) = 0;

    /** Looks up the given position in the book, and returns all the moves it found.
     *  If the position was not in the database, an empty vector is returned.
    */
    virtual QList<BookMove> LookupMoves(const char *fen) = 0;

    /** Closes the book, or does nothing if it's already closed. */
    virtual void CloseBook() = 0;

    virtual ~IBookReader(){}


    /** Used to catch progress updates during book validation. */
    class IValidationProgressObserver
    {
    public:
        virtual void OnValidationProgressUpdate(int) = 0;
    };

};


}

Q_DECLARE_INTERFACE(GKChess::IBookReader, "GKChess.IBookReader")

#endif // GKCHESS_IBOOKREADER_H
