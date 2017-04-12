#pragma once

#include <string>
#include <vector>
#include <ctype.h>
#include <algorithm>

using namespace std; //use for the sake of example

vector<string> split (const string &line)
{
    vector<string> ret;
    typedef string::const_iterator iter;
    iter i, j;

    i = line.begin();

    while (i != line.end()) {

        i = find_if(i, line.end(), [](char c){ return !isspace(c); });
        j = find_if(i, line.end(), [](char c){ return isspace(c); });

        if (i != line.end())
            ret.push_back(string(i,j));

        i=j;
    }
    return ret;
}

template <class write_iter>
void split(const string& line, write_iter destination)
{
    typedef string::const_iterator iter;
    iter i = line.begin();

    while(i != line.end())
    {
        i = find_if(i, line.end(), [](char c){ return !isspace(c); });
        iter j = find_if(i, line.end(), [](char c){ return isspace(c); });

        if (i != line.end())
            *destination = string(i,j);

        i = j;
    }
}
