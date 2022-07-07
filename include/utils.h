#ifndef ___HEADER_SWING_UTILS___
#define ___HEADER_SWING_UTILS___

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
#include<unordered_map>
#include <iterator>
#include <algorithm>


template <typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
    if (!v.empty()) {
        out << '[';
        std::copy(v.begin(), v.end(), std::ostream_iterator<T>(out, ", "));
        out << "\b\b]";
    }
    return out;
}

std::string currentTimetoStr(void);

void split(std::vector<std::string>& tokens, const std::string& s,  const std::string& delimiters = " ");

bool compare_pairs(const std::pair<int, float> & a, const std::pair<int, float> & b);


bool compare_i2ulist_map_iters(const std::unordered_map<int, std::vector<int> >::const_iterator & a, const std::unordered_map<int, std::vector<int> >::const_iterator & b);


#endif
