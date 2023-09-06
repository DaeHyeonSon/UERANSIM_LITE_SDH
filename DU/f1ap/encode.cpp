//
// Created by Hoonyong Park on 6/3/23.
//

#include "task.hpp"

#include <sstream>

using namespace std;

namespace nr::DU
{

std::string F1apTask::decode(const UniqueBuffer &buffer)
{
    char *tmp = new char[buffer.size()];

    for (int i = 0; i < (int)buffer.size(); i++)
    {
        tmp[i] = (buffer.data())[i];
    }

    tmp[buffer.size()] = '\0';

    std::string result(tmp);

    delete tmp;

    return result;
}

string F1apTask::Merge(vector<string> vec)
{
    string result = "";

    for (string &s : vec)
    {
        result = result + s + "|";
    }

    result = result.substr(0, result.length() - 1);

    return result;
}


vector<string> F1apTask::split(string input, char delimiter)
{
    vector<string> answer;
    stringstream ss(input);
    string temp;

    while (getline(ss, temp, delimiter))
    {
        answer.push_back(temp);
    }

    return answer;
}


}