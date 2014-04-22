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

#include "gkchess_chess960.h"
#include "gutil_file.h"
#include "gutil_set.h"
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GUTIL;

int main(int, char *[])
{
    File f("output.txt");
    f.Open(File::OpenWrite);
    
    Vector<String> res;
    for(int i = 0; i < 960; ++i)
        res.PushBack(Chess960::GetStartingPosition(i));
        
    if(0 == res.Length()){
        f.Write("ERROR: No Boards Generated\n");
        return -1;
    }
    else if(960 != res.Length()){
        f.Write("ERROR: Too few boards generated!\n");
    }
        
    // Check for duplicates in the return vector:
    {
        Set<String> test_set;
        for(int i = 0; i < res.Length(); ++i){
            if(test_set.Contains(res[i])){
                f.Write("ERROR: Return set contains duplicates!\n");
                break;
            }
            test_set.Insert(res[i]);
        }
    }
    
    for(int i = 0; i < res.Length(); ++i)
    {
        f.Write(String::Format("#%d. %s\n", i, res[i].ConstData()));
    }
    
    f.Write("\n");
    
    int indx;
    String pos = Chess960::GetRandomStartingPosition(&indx);
    f.Write(String::Format("Picking one at random (#%d):\n", indx));
    f.Write(pos);
    f.Write("\n\n");
    
    f.Write("This is the starting position for standard chess (#518):\n");
    f.Write(Chess960::GetStartingPosition(518));
    f.Write("\n");
    return 0;
}
