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
#include <tr1/functional>
#include <string.h>
//#include <unordered_map>
#include<tr1/unordered_map> // ? 
using namespace std::tr1;


//打印vector
#include <iterator> // for std::ostream_iterator
#include <algorithm>  // for std::copy

template <typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
    if (!v.empty()) {
        out << '[';
        std::copy(v.begin(), v.end(), std::ostream_iterator<T>(out, ", "));
        out << "\b\b]";
    }
    return out;
}


//#include "util/dict.hpp"
//#include "dict_fields.h"

using namespace std;

float threshold = 0.5;
float alpha = 0.5;

struct SPair {
  int a;
  int b;
  bool operator==(const SPair & other) const {
     return other.a == a && other.b == b;
  }
};

void split(vector<string>& tokens,const string& s,  const string& delimiters = " ")
{
    string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    string::size_type pos = s.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos) {
        tokens.push_back(s.substr(lastPos, pos - lastPos));//use emplace_back after C++11
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
}

int main(int argc,char *argv[]) {

    if (argc < 3) {
        cout << "usage " << argv[0] << " threshold alpha" << endl;
        return -1;
    }
    threshold = atof(argv[1]);
    alpha = atof(argv[2]);

    unordered_map<unsigned long, float> score_map;

    string line_buff;

    const string delimiters(",");

    vector<string> field_segs;
    vector< vector<int> > groups;
    groups.reserve(2000000);
    std::vector<int> item_list;
    
    std::vector<int> items_intersection_buffer;
    float vote = 0.0;
    unsigned long map_key = 0;
    unsigned long map_key_1 = 0;
    unsigned long map_key_2 = 0;

    while (getline(cin, line_buff))
    {
        //格式是一个json，所以要把开头和结尾的括号去掉
        line_buff.erase(0,line_buff.find_first_not_of("{"));
        line_buff.erase(line_buff.find_last_not_of("}") + 1);
        //cout << line_buff << " !!!" << endl;
        field_segs.clear();
        split(field_segs, line_buff, delimiters);

        item_list.clear();
        for (size_t i = 0; i < field_segs.size(); i++) {
            float value = atof(strchr(field_segs[i].c_str(), ':') + 1);
            if (value > threshold) {
                item_list.push_back(atoi(field_segs[i].c_str() + 1));
            }
            //cout << item_list[i] << " " << value << "|";
        }

        if (item_list.size() < 2) continue;
        // 排序

        sort(item_list.begin(), item_list.end());

        for (vector< vector<int> >::const_iterator iter = groups.begin(); iter != groups.end(); ++iter) {
            items_intersection_buffer.clear();
            set_intersection(item_list.begin(), item_list.end(), iter->begin(), iter->end(), back_inserter(items_intersection_buffer));//求交集
            //set_intersection(item_list.begin(), item_list.end(), iter->begin(), iter->end(), insert_iterator<vector<int> >(items_intersection_buffer,items_intersection_buffer.begin()));//求交集
            //cout << items_intersection_buffer.size() << " ";

            //如果交集大于2，则对集合内组成的pairs (j, k) 进行打分
            //
            if (items_intersection_buffer.size() > 1) {
                vote = 1.0 / (alpha + items_intersection_buffer.size());
                for (vector<int>::const_iterator j = items_intersection_buffer.begin() + 1;
                    j != items_intersection_buffer.end();
                    ++j) {
                        map_key_1 = (unsigned long)(*j);
                    for (vector<int>::const_iterator k = items_intersection_buffer.begin();
                        k != j;
                        ++k) {
                        map_key_2 = (unsigned long)(*k);
                        if (map_key_1 < map_key_2) map_key = (map_key_1<<32) + map_key_2;
                        else map_key = (map_key_2<<32) + map_key_1;

                        std::pair<unordered_map<unsigned long, float>::iterator, bool> ins_ret;
                        ins_ret = score_map.insert(make_pair(map_key, vote));
                        if (!ins_ret.second) {
                            ins_ret.first->second += vote;
                        }
                    }
                        
                }
            }
                    
            //cout << items_intersection_buffer << " ";
            //cout << endl;
        }
        // append本次的itemlist
        groups.push_back(item_list);

        //cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ " <<  endl;
    }

    for (unordered_map<unsigned long, float>::iterator iter=score_map.begin(); iter!=score_map.end(); ++iter) {
        int b1 = iter->first >> 32;
        int b2 = iter->first & 0xFFFFFFFF;
        cout << b1 << "\t" << b2 << "\t"  << iter->second << endl;
    }

    return 0;
}

