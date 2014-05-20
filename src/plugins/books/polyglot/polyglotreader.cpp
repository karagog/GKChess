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

#include "polyglotreader.h"
#include "gutil_macros.h"

#include "pg_utils/pg_utils.h"
#include <QtPlugin>
USING_NAMESPACE_GUTIL;


namespace{

struct d_t
{
    void *file;
    String filename;

    d_t() :file(0){}
};

}


namespace GKChess{


PolyglotBookReader::PolyglotBookReader(QObject *p)
    :QObject(p)
{
    G_D_INIT();
}

PolyglotBookReader::~PolyglotBookReader()
{
    CloseBook();
    G_D_UNINIT();
}

void PolyglotBookReader::OpenBook(const char *b)
{
    G_D;
    if(d->file)
        CloseBook();

    if(!(d->file = pg_open_file(b, 0)))
        THROW_NEW_GUTIL_EXCEPTION2(Exception,
                                   String::Format("Unable to open file: %s\n%s", b, pg_error_string()));

    d->filename = b;
}

bool PolyglotBookReader::IsBookOpen() const
{
    G_D;
    return d->file;
}

const char *PolyglotBookReader::GetBookFilename() const
{
    G_D;
    return d->filename;
}

void PolyglotBookReader::CloseBook()
{
    G_D;
    if(d->file)
    {
        pg_close_file(d->file);
        d->file = 0;
        d->filename.Empty();
    }
}


#define MAX_MOVES 50

Vector<PolyglotBookReader::Move> PolyglotBookReader::LookupMoves(const char *fen)
{
    G_D;
    Vector<Move> ret;
    if(d->file)
    {
        char tmps[6];
        pg_move_t moves[MAX_MOVES];
        unsigned int len = pg_lookup_moves(d->file, pg_compute_key(fen), moves, MAX_MOVES);
        
        for(unsigned int i = 0; i < len; ++i)
        {
            ret.PushBack(Move());

            pg_move_to_string(&moves[i], tmps);
            ret.Back().Text = tmps;

            ret.Back().Weight = moves[i].weight;
        }
    }
    return ret;
}


}

Q_EXPORT_PLUGIN2(polyglotReaderPlugin, GKChess::PolyglotBookReader)
