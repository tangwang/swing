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
#include <queue>
#include <math.h>
#include <numeric>
#include "utils.h"


int max_sim_list_len = 300;

using namespace std;

int main(int argc,char *argv[]) {

    float threshold1 = 0.5;
    float threshold2 = 0.5;
    int show_progress = 0;

    if (argc < 5) {
        cout << "usage " << argv[0] << " threshold1 threshold2 show_progress(0/1)" << endl;
        return -1;
    }


    threshold1 = atof(argv[1]);
    threshold2 = atof(argv[2]);
    show_progress = atoi(argv[3]);

    cerr << currentTimetoStr() << " start... " << endl;
    cerr << " threshold1 " << threshold1 << endl;
    cerr << " threshold2 " << threshold2 << endl;
    
    //一阶关系（DB簇索引）
    unordered_map<unsigned long, pair<int, float> > sim_by_1rs_relation_map(1000000);
    //sim_by_1rs_relation_map.reserve(1000000);

    string line_buff;

    const string delimiters(",");

    vector<string> field_segs;
    vector<pair<int, float> > item_list;
    
    while (getline(cin, line_buff)) {
         //格式是一个json，所以要把开头和结尾的括号去掉
        line_buff.erase(0,line_buff.find_first_not_of("{"));
        line_buff.erase(line_buff.find_last_not_of("}") + 1);
        //cout << line_buff << " !!!" << endl;
        field_segs.clear();
        split(field_segs, line_buff, delimiters);

        item_list.clear();
        for (size_t i = 0; i < field_segs.size(); i++) {
            const char * seg_pos = strchr(field_segs[i].c_str(), ':') ;
            if (seg_pos == NULL || (seg_pos - field_segs[i].c_str() >= field_segs[i].length())) break;

            float value = atof(seg_pos + 1);
            if (value > threshold1 || value > threshold2) {
                // 开头有一个双引号
                int item_id = atoi(field_segs[i].c_str() + 1);
                item_list.push_back(make_pair(item_id, value) );
            }
        }

        if (item_list.size() < 2) continue;
        // 排序
        //sort(item_list.begin(), item_list.end());
        
        // append本次的itemlist
        // 合入i2u索引
        unsigned long map_key = 0;
        unsigned long map_key_1 = 0;
        unsigned long map_key_2 = 0;
        pair<unordered_map<unsigned long, pair<int, float> >::iterator, bool> ins_ret;

        for (vector<pair<int, float> >::const_iterator i = item_list.begin(); i != item_list.end(); ++i) {
            map_key_1 = (unsigned long)(i->first);
            for (vector<pair<int, float> >::const_iterator j = item_list.begin(); j != item_list.end(); ++j) {
                map_key_2 = (unsigned long)(j->first);

                if (map_key_1 == map_key_2) continue;

                if (i->second > threshold1 && j->second > threshold2) {
                    map_key = (map_key_1<<32) + map_key_2;
                    ins_ret = sim_by_1rs_relation_map.insert(make_pair(map_key, make_pair(1, j->second) ));
                    if (!ins_ret.second) {
                        ins_ret.first->second.first += 1;
                        ins_ret.first->second.second+= j->second;
                    }
                }
                if (j->second > threshold1 && i->second > threshold2) {
                    map_key = (map_key_2<<32) + map_key_1;
                    ins_ret = sim_by_1rs_relation_map.insert(make_pair(map_key, make_pair(1, i->second) ));
                    if (!ins_ret.second) {
                        ins_ret.first->second.first += 1;
                        ins_ret.first->second.second+= i->second;
                    }
                }
            }
        }
    }

    unordered_map<int, vector<pair<int, float> > > sim_matrix(200000);
    // 计算item_i, item_j合并的打分，total_wei / num * math.log(1.5*num, 1.5). 
    pair<int, vector<pair<int, float> > > pair_entry;
    pair<unordered_map<int, vector<pair<int, float> > >::iterator, bool> ins_ret;

    for (unordered_map<unsigned long, pair<int, float> >::iterator iter=sim_by_1rs_relation_map.begin(); iter!=sim_by_1rs_relation_map.end(); ++iter) {
        int item1 = iter->first >> 32;
        int item2 = iter->first & 0xFFFFFFFF;

        int num = iter->second.first;
        float total_wei = iter->second.second;
        float merged_score = total_wei / num * log(1.5*num);

        pair_entry.first = item1;

        ins_ret = sim_matrix.insert(pair_entry);
        ins_ret.first->second.push_back(make_pair(item2, merged_score));
    }

    // staits info of sim matrix
    vector<int> sim_list_len_statis;
    sim_list_len_statis.resize(max_sim_list_len+1);
    
    // write sim matrix
    for (unordered_map<int, vector<pair<int, float> > >::iterator iter=sim_matrix.begin(); iter!=sim_matrix.end(); ++iter) {
        vector<pair<int, float> > & sim_list_buff = iter->second;
        int sim_list_len = sim_list_buff.size();
        if (sim_list_len > 0) {
            sort(sim_list_buff.begin(), sim_list_buff.end(), compare_pairs);

            cout << iter->first << "\t" << sim_list_buff[0].first << ":" << sim_list_buff[0].second;

            if (sim_list_len > max_sim_list_len) sim_list_len = max_sim_list_len;

            sim_list_len_statis[sim_list_len] += 1;

            for (int i = 1; i < sim_list_len; i++) {
                cout << ',' << sim_list_buff[i].first << ':' << sim_list_buff[i].second;
            }
            cout << endl; 
        }
    }

    // staits info of sim matrix
    int sum_groups = accumulate(sim_list_len_statis.begin(), sim_list_len_statis.end(), (int)0);
    cerr << currentTimetoStr() <<  " write sim matrix finished" << endl;
    cerr << currentTimetoStr() <<  " print staits info of sim matrix... " << sim_list_len_statis.size() << endl;
    cerr << currentTimetoStr() <<  " total keys: " << sum_groups  << endl;

    int accumulate = 0;
    for (int i = sim_list_len_statis.size()-1; i > -1; i--) {
        accumulate += sim_list_len_statis[i];
        fprintf(stderr, "simlist_len %4d, num %4d, accumulate %6d accumulated_rate %5.2f%\%\n", (int)i, sim_list_len_statis[i], accumulate, 100.0*accumulate/sum_groups);
    }

    return 0;
}
