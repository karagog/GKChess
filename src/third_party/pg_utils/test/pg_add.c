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
    char *new_book;
    FILE *b;
    FILE *n;
    entry_t entry, entry1;
    board_t board;
    uint64 key;
    int count,max_weight,i;
    entry_t entries[MAX_MOVES];
    double scale;
    char *move_s;
    uint16 move;
    bool move_new, entry_queued;
    if(argc<5){
      fprintf(stderr,"Usage: pg_add <source> \"<fen>\" <move> <dest>\n");
        return 1;
    }
    book=argv[1];
    fen=argv[2];
    move_s=argv[3];
    new_book=argv[4];
    if(!strcmp(book,new_book)){
        fprintf(stderr,"pg_add: Source and destination cannot be the same.\n");
	return 1;
    }
    if(move_from_string(move_s,&move)){
        fprintf(stderr,"%s: Illegal move\n",move_s);
        return 1;
    }
    if(board_from_fen(&board,fen)){
        fprintf(stderr,"%s: Illegal FEN\n",fen);
        return 1;
    }
    key=hash(&board);
    if(!(b=fopen(book,"rb"))){
        perror(book);
        return 1;
    }
    if(!(n=fopen(new_book,"wb"))){
        perror(new_book);
        fclose(b);
        return 1;
    }
    while(!entry_from_file(b,&entry)){
        if(key!=entry.key){
            entry_to_file(n,&entry);
            continue;
        }else{
            break;
        }
    }
    if(entry.key!=key){
        fprintf(stderr,"%s: No such fen in \"%s\"\n", fen,book);
        fclose(b);
        fclose(n);
        return 1;
    }
    entries[0]=entry;
    count=1;
    entry_queued=0;
    while(1){
        if(entry_from_file(b,&entry)){
	    break;
        }
        if(entry.key!=key){
            entry_queued=1;
            break;
        }
        if(count==MAX_MOVES){
	    fprintf(stderr,"pg_add: Too many moves in this position (max=%d)\n",MAX_MOVES);
            fclose(b);
            fclose(n);
            return 1;
        }
        entries[count++]=entry;
    }
        // move already there?
    move_new=1;
    for(i=0;i<count;i++){
        if(entries[i].move==move){
	    fprintf(stderr,"%s: Move already present (ignoring)\n",move_s);
            move_new=0;
        }
    }
        // rescaling
    max_weight=0;
    for(i=0;i<count;i++){
        if(entries[i].weight>max_weight){
            max_weight=entries[i].weight;
        }
    }
        // theoretically we prefer 0xffff but we stay on the safe side
    scale=((double) 0xfff0)/((double) max_weight);
    for(i=0;i<count;i++){
        if(move_new){
            entries[i].weight=(int)( scale* ((double) entries[i].weight));
        }
        entry_to_file(n,entries+i);
    }
    if(move_new){
        entry1.key=key;
        entry1.move=move;
        entry1.weight=1;
        entry1.learn=0;
        entry_to_file(n,&entry1);
    }
    if(entry_queued){
        entry_to_file(n,&entry);
        while(!entry_from_file(b,&entry)){
            entry_to_file(n,&entry);
        }
    }
    
    fclose(b);
    fclose(n);
    return 0;
}

