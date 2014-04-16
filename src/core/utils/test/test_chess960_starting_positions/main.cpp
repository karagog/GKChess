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
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GUTIL;

int main(int, char *[])
{
    File f("output.txt");
    f.Open(File::OpenWrite);
    
    Vector<String> res = Chess960::GetAllStartingPositions();
    if(0 == res.Length()){
        f.Write("No Boards Generated\n");
        return -1;
    }
        
    for(int i = 0; i < res.Length(); ++i)
    {
        f.Write(String::Format("#%d. %s\n", i+1, res[i].ConstData()));
    }
    
    f.Write("\n");
    
    f.Write("Picking one at random:\n");
    f.Write(Chess960::GetRandomStartingPosition());
    f.Write("\n");
    return 0;
}
