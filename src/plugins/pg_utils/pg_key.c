#include <stdio.h>
#include <string.h>

#include "hash.h"
#include "board.h"

int main(int argc, char *argv[]){
    board_t board;
    char *fen;
    uint64 key;
    if(argc==1){
        fprintf(stderr,"Usage:  pg_key \"<fen>\"\n");
        return 1;
    }
    fen=argv[1];
    if(board_from_fen(&board,fen)){
        fprintf(stderr,"%s: Illegal FEN\n",fen);
        return 1;
    }
    key=hash(&board);
    /* avoid compiler long long weirdness */
    printf("%08x",(uint32)(key>>32));
    printf("%08x\n",(uint32)(key&0xffffffff));
    return 0;
}
