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

static SmartPointer<Vector<String> > __chess960_positions;

NAMESPACE_GKCHESS;


static void __place_pieces(Set<String> &results, 
                           Vector<char> &placed_pieces,
                           IStack<char> &unplaced_pieces,
                           int &bishop_state,
                           int &king_index)
{
    // Termination condition:
    if(0 == unplaced_pieces.Size())
    {
        // There are no more pieces to place, so add the pieces to the results
        String tmp(8);
        for(int i = 0; i < placed_pieces.Length(); ++i)
            tmp.Append(placed_pieces[i]);
        
        // With this implementation we generate lots of duplicates, so here is where we filter them
        if(!results.Contains(tmp))
            results.Insert(tmp);
            
        return;
    }
        
    // Grab the current piece from the stack
    char cur_piece = unplaced_pieces.Top();
    unplaced_pieces.Pop();
    const int bishop_state_orig = bishop_state;
    
    // Try placing the piece in all possible positions
    for(int i = 0; i < 8; ++i)
    {
        bool place = false;
        char &target_piece( placed_pieces[i] );
        
        // Skip this square if there is already a piece there
        if(' ' != target_piece)
            continue;

        // There are rules for the positions of certain pieces
        switch(cur_piece)
        {
        case 'K':
            // The king can only be placed between cols 1 and 6
            if(0 < i && i < 7){
                place = true;
                king_index = i;
            }
            break;
        case 'R':
            if('R' == unplaced_pieces.Top() && i < king_index){
                // If this is the first rook, put it on the left side of the king
                place = true;
            }
            else if('R' != unplaced_pieces.Top() && i > king_index){
                // If this is the second rook, put it on the right side of the king
                place = true;
            }
            break;
        case 'B':
            // If this is the first bishop being placed...
            if(0 == bishop_state){
                place = true;
                bishop_state = 0x2 | (0x1 & i);
            }
            
            // If this is the second bishop being placed, and the first was
            //  on a light square and this is a dark square...
            else if((1 == (0x1 & bishop_state)) && 
                    (0 == (0x1 & i))){
                place = true;
            }
            
            // If this is the second bishop being placed, and the first
            //  was on a dark square and this is a light square...
            else if((0 == (0x1 & bishop_state)) && 
                    (1 == (0x1 & i))){
                place = true;
            }
            break;
        case 'N':
            place = true;
            break;
        case 'Q':
            place = true;
            break;
        default: 
            GASSERT(false);
        }
        
        // If we placed a piece, then recursively call again to place the others
        if(place)
        {
            target_piece = cur_piece;
            __place_pieces(results, placed_pieces, unplaced_pieces, bishop_state, king_index);
            
            // Reset the piece before placing it someplace new
            target_piece = ' ';
            bishop_state = bishop_state_orig;
        }
    }
    
    // Push the piece back on the stack when we're finished
    unplaced_pieces.Push(cur_piece);
}

Vector<String> const &Chess960::GetAllStartingPositions()
{
    if(!__chess960_positions)
    {
        // Generate all positions the first time
        
        // Allocate the static memory that holds the position data
        __chess960_positions = new Vector<String>;
        __chess960_positions->ReserveExactly(960);
        
        // Instantiate temporary variables
        Set<String> results;
        Vector<char> pp(' ', 8);
        Vector<char, IStack<char> > up(8);
        int king_index;
        int bishop_state = 0;
        
        // These are all the pieces that need to be placed (the order matters)
        up.Push('Q');
        up.Push('N');
        up.Push('N');
        up.Push('B');
        up.Push('B');
        up.Push('R');
        up.Push('R');
        up.Push('K');
        
        // Call the recursive function that places the pieces in all configurations
        __place_pieces(results, pp, up, bishop_state, king_index);
        
        // Now iterate through the results and generate the FEN strings
        G_FOREACH_CONST(const String &s, results)
        {
            __chess960_positions->PushBack(
                String::Format("%s/pppppppp/8/8/8/8/PPPPPPPP/%s w KQkq - 0 1",
                    s.ToLower().ConstData(), s.ConstData())
            );
        }
    }
    
    return *__chess960_positions;
}

String const &Chess960::GetRandomStartingPosition()
{
    Vector<String> const &vec(GetAllStartingPositions());
    return vec[RNG::RandInt(0, vec.Length() - 1)];
}


END_NAMESPACE_GKCHESS;
