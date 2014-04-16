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

#include "rng.h"
#include "chess960.h"
#include "gutil_smartpointer.h"
#include "gutil_set.h"
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;


static Vector<int> __get_unused_indices(const Set<int> &inds)
{
    Vector<int> ret;
    ret.ReserveExactly(inds.Size());
    G_FOREACH_CONST(int i, inds)
        ret.PushBack(i);
    return ret;
}

#define FEN_STARTING_FORMAT_STRING "%s/pppppppp/8/8/8/8/PPPPPPPP/%s w KQkq - 0 1"

static void __place_king_and_rooks(Vector<String> &results,
                            String &placed_pieces,
                            Set<int> &unused_indices)
{
    Vector<int> ui(__get_unused_indices(unused_indices));

    // There are exactly 3 indices left, and the set sorted them in ascending order
    placed_pieces[ui[0]] = 'R';
    placed_pieces[ui[1]] = 'K';
    placed_pieces[ui[2]] = 'R';

    // Add the current configuration to the results
    results.PushBack(String::Format(FEN_STARTING_FORMAT_STRING,
                                    placed_pieces.ToLower().ConstData(),
                                    placed_pieces.ConstData()));

    placed_pieces[ui[0]] = ' ';
    placed_pieces[ui[1]] = ' ';
    placed_pieces[ui[2]] = ' ';
}

static void __iteratively_place_queen(Vector<String> &results,
                               String &placed_pieces,
                               Set<int> &unused_indices)
{
    Vector<int> ui(__get_unused_indices(unused_indices));
    for(int i = 0; i < ui.Length(); ++i)
    {
        unused_indices.RemoveOne(ui[i]);

        placed_pieces[ui[i]] = 'Q';
        __place_king_and_rooks(results, placed_pieces, unused_indices);

        placed_pieces[ui[i]] = ' ';
        unused_indices.Insert(ui[i]);
    }
}

static void __iteratively_place_knights(Vector<String> &results,
                                 String &placed_pieces,
                                 Set<int> &unused_indices)
{
    Vector<int> ui(__get_unused_indices(unused_indices));
    for(int i = 0; i < ui.Length(); ++i)
    {
        for(int j = i + 1; j < ui.Length(); ++j)
        {
            unused_indices.RemoveOne(ui[i]);
            unused_indices.RemoveOne(ui[j]);

            placed_pieces[ui[i]] = 'N';
            placed_pieces[ui[j]] = 'N';
            __iteratively_place_queen(results, placed_pieces, unused_indices);

            placed_pieces[ui[i]] = ' ';
            placed_pieces[ui[j]] = ' ';
            unused_indices.Insert(ui[i]);
            unused_indices.Insert(ui[j]);
        }
    }
}

static void __iteratively_place_bishops(Vector<String> &results,
                                 String &placed_pieces,
                                 Set<int> &unused_indices)
{
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            int ind1 = i * 2;
            int ind2 = j * 2 + 1;
            unused_indices.RemoveOne(ind1);
            unused_indices.RemoveOne(ind2);

            placed_pieces[ind1] = 'B';
            placed_pieces[ind2] = 'B';
            __iteratively_place_knights(results, placed_pieces, unused_indices);

            placed_pieces[ind1] = ' ';
            placed_pieces[ind2] = ' ';
            unused_indices.Insert(ind1);
            unused_indices.Insert(ind2);
        }
    }
}






Vector<String> Chess960::GetAllStartingPositions()
{
    Vector<String> ret;
    ret.ReserveExactly(960);

    // Instantiate temporary variables
    String pp(' ', 8);
    Set<int> ui;
    for(int i = 0; i < 8; ++i)
        ui.Insert(i);

    __iteratively_place_bishops(ret, pp, ui);
    return ret;
}

static void __place_piece(String &ret, Vector<int> &unused_indices, char piece)
{
    int i = RNG::RandInt(0, unused_indices.Length() - 1);
    ret[unused_indices[i]] = piece;
    unused_indices.RemoveAt(i);
}

String Chess960::GetRandomStartingPosition()
{
    // We will generate one on the fly
    String ret(' ', 8);
    Vector<int> unused_indices(8);
    for(int i = 0; i < 8; ++i)
        unused_indices.PushBack(i);

    // First place the bishops
    int ind1 = RNG::RandInt(0, unused_indices.Length() / 2 - 1) * 2;
    int ind2 = RNG::RandInt(0, unused_indices.Length() / 2 - 1) * 2 + 1;
    ret[unused_indices[ind1]] = 'B';
    ret[unused_indices[ind2]] = 'B';
    unused_indices.RemoveOne(Max(ind1, ind2));
    unused_indices.RemoveOne(Min(ind1, ind2));

    // Then place the knights
    __place_piece(ret, unused_indices, 'N');
    __place_piece(ret, unused_indices, 'N');

    // Then place the queen
    __place_piece(ret, unused_indices, 'Q');

    // Then there are three spots left and our rooks have to be on either side of our king
    ret[unused_indices[0]] = 'R';
    ret[unused_indices[1]] = 'K';
    ret[unused_indices[2]] = 'R';

    return String::Format(FEN_STARTING_FORMAT_STRING,
                         ret.ToLower().ConstData(),
                         ret.ConstData());
}


END_NAMESPACE_GKCHESS;
