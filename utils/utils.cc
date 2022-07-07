#include <iostream>
#include <fstream>
#include <utility>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <functional>
#include <string.h>
#include <time.h>
//#include <unordered_map>
#include<unordered_map>

#include <iterator>
#include <algorithm>
void split(std::vector<std::string>& tokens, const std::string& s,  const std::string& delimiters = " ")
{
    using namespace std;
    string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    string::size_type pos = s.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos) {
        tokens.push_back(s.substr(lastPos, pos - lastPos));//use emplace_back after C++11
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
}

std::string currentTimetoStr(void) {
    char tmp[64];
    time_t t = time(NULL);
    tm *_tm = localtime(&t);
    int year  = _tm->tm_year+1900;
    int month = _tm->tm_mon+1;
    int date  = _tm->tm_mday;
    int hh = _tm->tm_hour;
    int mm = _tm->tm_min;
    int ss = _tm->tm_sec;
    sprintf(tmp,"%04d-%02d-%02d %02d:%02d:%02d", year,month,date,hh,mm,ss);
    return std::string(tmp);
}


bool compare_i2ulist_map_iters(const std::unordered_map<int, std::vector<int> >::const_iterator & a, const std::unordered_map<int, std::vector<int> >::const_iterator & b) {
    // vector长的排序后面
    return a->second.size() < b->second.size();
}

bool compare_pairs(const std::pair<int, float> & a, const std::pair<int, float> & b) {
    // 分数大的排前面
    return a.second > b.second;
}

