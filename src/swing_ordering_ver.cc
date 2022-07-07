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
#include <time.h>
//#include <unordered_map>
#include<tr1/unordered_map>
#include <iterator>
#include <algorithm>
#include "utils.h"


int max_sim_list_len = 400;

using namespace std::tr1;
using namespace std;

int main(int argc,char *argv[]) {

    float threshold = 0.5;
    float alpha = 0.5;
    int show_progress = 0;

    if (argc < 4) {
        cout << "usage " << argv[0] << " threshold alpha show_progress(to_stderr)(0/1)" << endl;
        return -1;
    }
    threshold = atof(argv[1]);
    alpha = atof(argv[2]);
    show_progress = atoi(argv[3]);

    unordered_map<int, vector<int> > i2u_map;

    map<unsigned long, float> local_map;

    string line_buff;

    const string delimiters(",");

    vector<string> field_segs;
    vector< vector<int> > groups;
    groups.reserve(2000000);
    std::vector<int> item_list;
    
    std::vector<int> items_intersection_buffer;
    std::vector<int> users_intersection_buffer;
    users_intersection_buffer.reserve(2000);
    unsigned long map_key = 0;
    unsigned long map_key_1 = 0;
    unsigned long map_key_2 = 0;

    while (getline(cin, line_buff)) {
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
                int item_id = atoi(field_segs[i].c_str() + 1);
                item_list.push_back(item_id);
            }
           //cout << item_list[i] << " " << value << "|";
        }

        if (item_list.size() < 2) continue;
        // 排序

        sort(item_list.begin(), item_list.end());
        
        // append本次的itemlist
        groups.push_back(item_list);
        int idx = groups.size();
        // 合入i2u索引
        for (vector<int>::const_iterator iter = item_list.begin(); iter != item_list.end(); ++iter) {
            std::pair<unordered_map<int, vector<int> >::iterator, bool> ins_i2u_ret;
            std::pair<int, vector<int> > pair_entry;
            pair_entry.first = *iter;
            ins_i2u_ret = i2u_map.insert(pair_entry);
            ins_i2u_ret.first->second.push_back(idx);
        }
    }

    int items_num = i2u_map.size();
    int users_num = groups.size();
    cerr << currentTimetoStr() << "items num: " << i2u_map.size() << endl;
    cerr << currentTimetoStr() << "users num: " << groups.size() << endl;
    cerr << currentTimetoStr() <<  "sort.." << endl;
    for (unordered_map<int, vector<int> >::iterator iter = i2u_map.begin(); iter != i2u_map.end(); ++iter) {
        sort(iter->second.begin(), iter->second.end());
    }
    cerr << currentTimetoStr() << "sort finished" << endl;

    if (items_num < 2) return 0;

    vector<pair<int, float> > sim_list_buff;

    int idx = 0;

    unordered_map<int, vector<int> >::const_iterator pair_i = i2u_map.begin();
    pair_i++;
    // TODO item_i 的 user_list 可以用unordered_set存起来，然后遍历item_j的user_list，看是否在set_of_item_i_users中
    for (; pair_i != i2u_map.end(); ++pair_i) {
        if (show_progress) {
            fprintf(stderr, "\r%d of %d", idx++, items_num);
        }
        sim_list_buff.clear();
        for (unordered_map<int, vector<int> >::const_iterator pair_j = i2u_map.begin(); pair_j != pair_i; ++pair_j) {
            users_intersection_buffer.clear();
            // 交互过item_i, item_j的user_list
            set_intersection(pair_i->second.begin(), pair_i->second.end(), pair_j->second.begin(), pair_j->second.end(), back_inserter(users_intersection_buffer));

            if (users_intersection_buffer.size() < 2) continue;
            // user_i, user_j

            float sim_of_item_i_j = 0.0;
            // 遍历共同交互过(item_i, item_j)的user组合(user_i, user_j)
            for (vector<int>::const_iterator user_i = users_intersection_buffer.begin()+1;
                user_i != users_intersection_buffer.end();
                ++user_i) {

                const vector<int> & item_list_of_user_i = groups[*user_i];

                for (vector<int>::const_iterator user_j = users_intersection_buffer.begin();
                    user_j != user_i;
                    ++user_j) {
                    
                    const vector<int> & item_list_of_user_j = groups[*user_j];
                    items_intersection_buffer.clear();
                    // user_i, user_j交互过的items的交集
                    set_intersection(item_list_of_user_i.begin(), item_list_of_user_i.end(), item_list_of_user_j.begin(), item_list_of_user_j.end(), back_inserter(items_intersection_buffer));//求交集

                    sim_of_item_i_j += 1.0 / (alpha + items_intersection_buffer.size());
                }
            }
            sim_list_buff.push_back(make_pair(pair_j->first, sim_of_item_i_j));
        }

        int sim_list_len = sim_list_buff.size();
        if (sim_list_len > 0) {
            sort(sim_list_buff.begin(), sim_list_buff.end(), compare_pairs);

            cout << pair_i->first << "\t" << sim_list_buff[0].first << ":" << sim_list_buff[0].second;

            if (sim_list_len > max_sim_list_len) sim_list_len = max_sim_list_len;
            for (int i = 1; i < sim_list_len; i++) {

                cout << ',' << sim_list_buff[i].first << ':' << sim_list_buff[i].second;
            }
            cout << endl; 
        }
    }
    cerr << currentTimetoStr() <<  "write sim matrix finished" << endl;
    return 0;
}
