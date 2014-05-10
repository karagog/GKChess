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
#include <stdio.h>

#include "pg_utils/book.h"
#include "pg_utils/move.h"
#include "pg_utils/hash.h"
#include "pg_utils/board.h"
#include <QtPlugin>
USING_NAMESPACE_GUTIL;


namespace{

struct d_t
{
    FILE *file;
    String filename;

    d_t() :file(0){}
};

}


namespace GKChess{


PolyglotBookReader::PolyglotBookReader()
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

    if(!(d->file = fopen(b, "r")))
        THROW_NEW_GUTIL_EXCEPTION2(Exception, String::Format("Unable to open file: %s", b));

    d->filename = b;
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
        fclose(d->file);
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
        board_t board;
        if(0 == board_from_fen(&board,fen))
        {
            uint64 key=hash(&board);
            entry_t entry;
            int offset=find_key(d->file,key,&entry);
            if(entry.key == key)
            {
                entry_t entries[MAX_MOVES];
                entries[0]=entry;
                int count=1;
                fseek(d->file,16*(offset+1),SEEK_SET);
                G_FOREVER{
                    if(entry_from_file(d->file,&entry)){
                        break;
                    }
                    if(entry.key!=key){
                        break;
                    }
                    if(count==MAX_MOVES){
                        break;
                    }
                    entries[count++]=entry;
                }

                int total_weight=0;
                for(int i = 0;i<count;i++)
                    total_weight+=entries[i].weight;

                for(int i = 0;i<count;i++)
                {
                    char move_s[6];
                    move_to_string(move_s,entries[i].move);

                    Move m;
                    m.Text = move_s;
                    m.Weight = (float) entries[i].weight / total_weight * 100;
                    ret.PushBack(m);
                }
            }
        }
    }
    return ret;
}


}
