#pragma once
#include <vector>
#include <string>

using namespace std;


string::size_type widthString(const vector<string>& v)
{
    string::size_type maxlen = 0;
    for(vector<string>::size_type i = 0; i < v.size(); ++i)
        maxlen = max(v[i].size(), maxlen);

    return maxlen;
}

vector<string> frame(const vector<string>& v)
{
    vector<string> ret;
    string::size_type maxlen = widthString(v);
    string border(maxlen + 4, '*');

    ret.push_back(border);

    for(vector<string>::size_type i = 0; i < v.size(); ++i)
        ret.push_back("* " + v[i] + string(maxlen - v[i].size(), ' ') + " *");

    ret.push_back(border);

    return ret;
}
