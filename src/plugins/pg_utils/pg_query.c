#include <stdio.h>
#include <string.h>

#include "book.h"
#include "move.h"
#include "hash.h"
#include "board.h"

#define MAX_MOVES 100

int main(int argc, char *argv[]){
    char *book;
    char *fen;
    FILE *b;
    entry_t entry;
    board_t board;
    int count,offset,total_weight,i;
    entry_t entries[MAX_MOVES];
    uint64 key;
    char move_s[6];
    if(argc<3){
        fprintf(stderr,"Usage: pg_query <book> \"<fen>\"\n");
        return 1;
    }
    book=argv[1];
    fen=argv[2];
    if(board_from_fen(&board,fen)){
        fprintf(stderr,"%s: Illegal FEN\n",fen);
        return 1;
    }
    key=hash(&board);
    if(!(b=fopen(book,"rb"))){
        perror(book);
        return 1;
    }
    offset=find_key(b,key,&entry);
    if(entry.key!=key){
        fprintf(stderr,"%s: No such fen in \"%s\"\n", fen,book);
        fclose(b);
        return 1;
    }
    entries[0]=entry;
    count=1;
    fseek(b,16*(offset+1),SEEK_SET);
    while(TRUE){
        if(entry_from_file(b,&entry)){
            break;
        }
        if(entry.key!=key){
            break;
        }
        if(count==MAX_MOVES){
	    fprintf(stderr,"pg_query: Too many moves in this position (max=%d)\n",MAX_MOVES);
            return 1;
        }
        entries[count++]=entry;
    }
    total_weight=0;
    for(i=0;i<count;i++){
        total_weight+=entries[i].weight;
    }
    for(i=0;i<count;i++){
        move_to_string(move_s,entries[i].move);
        printf("move=%s weight=%5.2f%%\n",
                move_s,
               100*((double) entries[i].weight/ (double) total_weight));
    }
    return 0;
}
