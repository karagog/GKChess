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

#ifndef GKCHESS_POLYGLOTREADER
#define GKCHESS_POLYGLOTREADER

#include "gkchess_ibookreader.h"

namespace GKChess{


/** A plugin that reads a book in polyglot format.
 *  See the interface documentation for more info.
*/
class PolyglotBookReader :
        public QObject,
        public IBookReader
{
    Q_OBJECT
    Q_INTERFACES(GKChess::IBookReader)
    Q_PLUGIN_METADATA(IID "GKChess.PolyglotReader")
    void *d;
public:

    PolyglotBookReader(QObject * = 0);
    ~PolyglotBookReader();

    void OpenBook(const char *);
    bool IsBookOpen() const;
    const char *GetBookFilename() const;
    void CloseBook();
    QList<BookMove> LookupMoves(const char *fen);

    /** \note You can only observe the progress for one validation globally at a time. */
    void ValidateBook(IValidationProgressObserver *);

};


}

#endif // GKCHESS_POLYGLOTREADER
