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

#include <gutil/rng.h>
#include "chess960.h"
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;


GBYTE __bishop_lut[] =
{
    0b11000000,
    0b10010000,
    0b10000100,
    0b10000001,
    0b01100000,
    0b00110000,
    0b00100100,
    0b00100001,
    0b01001000,
    0b00011000,
    0b00001100,
    0b00001001,
    0b01000010,
    0b00010010,
    0b00000110,
    0b00000011
};

char __king_lut[][7] =
{
    "QNNRKR", "NQNRKR", "NNQRKR", "NNRQKR", "NNRKQR", "NNRKRQ",
    "QNRNKR", "NQRNKR", "NRQNKR", "NRNQKR", "NRNKQR", "NRNKRQ",
    "QNRKNR", "NQRKNR", "NRQKNR", "NRKQNR", "NRKNQR", "NRKNRQ",
    "QNRKRN", "NQRKRN", "NRQKRN", "NRKQRN", "NRKRQN", "NRKRNQ",
    "QRNNKR", "RQNNKR", "RNQNKR", "RNNQKR", "RNNKQR", "RNNKRQ",
    "QRNKNR", "RQNKNR", "RNQKNR", "RNKQNR", "RNKNQR", "RNKNRQ",
    "QRNKRN", "RQNKRN", "RNQKRN", "RNKQRN", "RNKRQN", "RNKRNQ",
    "QRKNNR", "RQKNNR", "RKQNNR", "RKNQNR", "RKNNQR", "RKNNRQ",
    "QRKNRN", "RQKNRN", "RKQNRN", "RKNQRN", "RKNRQN", "RKNRNQ",
    "QRKRNN", "RQKRNN", "RKQRNN", "RKRQNN", "RKRNQN", "RKRNNQ"
};


static String __generate_position(int indx)
{
    String ret(8);
    GBYTE bishop_vals = __bishop_lut[indx & 0x0F];
    const char *other_vals = __king_lut[indx >> 4];
    int other_indx = 0;
    for(int i = 0; i < 8; ++i){
        if(bishop_vals & (((GUINT8)0b10000000) >> i))
            ret.Append('B');
        else{
            ret.Append(other_vals[other_indx]);
            ++other_indx;
        }
    }
    return ret;
}

String Chess960::GetStartingPosition(int indx)
{
    String tmp;
    if(0 <= indx && indx < 960)
    {
        tmp = __generate_position(indx);
        tmp = String::Format("%s/pppppppp/8/8/8/8/PPPPPPPP/%s w KQkq - 0 1",
                             tmp.ToLower().ConstData(),
                             tmp.ConstData());
    }
    return tmp;
}

String Chess960::GetRandomStartingPosition(int *indx)
{
    int tmpindx = GlobalRNG()->U_Discrete(0, 959);
    if(indx)
        *indx = tmpindx;
    return GetStartingPosition(tmpindx);
}


END_NAMESPACE_GKCHESS;
