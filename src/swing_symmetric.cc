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
#include <numeric>
#include "utils.h"
#include "BitMap.h"


int max_sim_list_len = 300;

using namespace std;

int main(int argc,char *argv[]) {

    float alpha = 0.5;
    float threshold = 0.5;
    int show_progress = 0;

    if (argc < 4) {
        cerr << "usage " << argv[0] << " alpha threshold show_progress(0/1)" << endl;
        return -1;
    }

    alpha = atof(argv[1]);
    threshold = atof(argv[2]);
    show_progress = atoi(argv[3]);
    
    cerr << currentTimetoStr() << " start... " << endl;
    cerr << " alpha " << alpha << endl;
    cerr << " threshold " << threshold << endl;

    unordered_map<int, vector<int> > i2u_map;
    i2u_map.reserve(160000);

    string line_buff;

    const string delimiters(",");

    vector<string> field_segs;
    vector< vector<int> > groups;
    groups.reserve(2000000);
    vector<int> item_list;
    
    vector<int> items_intersection_buffer;
    vector<int> users_intersection_buffer;
    users_intersection_buffer.reserve(2000);

    pair<int, vector<int> > pair_entry;
    pair<unordered_map<int, vector<int> >::iterator, bool> ins_i2u_ret;

    while (getline(cin, line_buff)) {
         //格式是一个json，所以要把开头和结尾的括号去掉
        line_buff.erase(0,line_buff.find_first_not_of("{"));
        line_buff.erase(line_buff.find_last_not_of("}") + 1);
        field_segs.clear();
        split(field_segs, line_buff, delimiters);

        item_list.clear();
        for (size_t i = 0; i < field_segs.size(); i++) {
            const char * seg_pos = strchr(field_segs[i].c_str(), ':') ;
            if (seg_pos == NULL || (seg_pos - field_segs[i].c_str() >= field_segs[i].length())) break;

            float value = atof(seg_pos + 1);
            if (value > threshold) {
                // 开头有一个双引号
                int item_id = atoi(field_segs[i].c_str() + 1);
                item_list.push_back(item_id);
            }
        }

        if (item_list.size() < 2) continue;
        // 排序
        sort(item_list.begin(), item_list.end());
        
        // append本次的itemlist
        int idx = groups.size(); // 下标应该是待插入的位置，20210805 之前错了！
        groups.push_back(item_list);
        // 合入i2u索引
        for (vector<int>::const_iterator iter = item_list.begin(); iter != item_list.end(); ++iter) {
            pair_entry.first = *iter;
            ins_i2u_ret = i2u_map.insert(pair_entry);
            ins_i2u_ret.first->second.push_back(idx);
        }
    }

    int items_num = i2u_map.size();
    int users_num = groups.size();
    cerr << currentTimetoStr() << " items num: " << i2u_map.size() << endl;
    cerr << currentTimetoStr() << " users num: " << groups.size() << endl;
    cerr << currentTimetoStr() <<  " sort.." << endl;


    // TODO 对i2u_map排序，list长的 放后面，永远是后者转set，前者遍历，相比于不排序性能提升一倍多
    // 而对所有热门item都转set的话，会消耗较多内存
    vector<unordered_map<int, vector<int> >::const_iterator> sorted_i_ulist_pairs;

    for (unordered_map<int, vector<int> >::iterator iter = i2u_map.begin(); iter != i2u_map.end(); ++iter) {
        sorted_i_ulist_pairs.push_back(iter);
        sort(iter->second.begin(), iter->second.end());
    }
    cerr << currentTimetoStr() << " sort finished" << endl;
    
    sort(sorted_i_ulist_pairs.begin(), sorted_i_ulist_pairs.end(), compare_i2ulist_map_iters);

    if (items_num < 2) return -1;

    vector<pair<int, float> > sim_list_buff;
    unordered_map<int, vector<pair<int, float> > > sim_matrix;
    sim_matrix.reserve(items_num);

    int idx = 0;

    BitMap user_bm(users_num);
    bool use_bitmap;
    vector<int> sim_list_len_statis;
    sim_list_len_statis.resize(max_sim_list_len+1);

    // TODO 热门item_i 的 user_list 可以用std::set存起来
    for (int i = 1; i < sorted_i_ulist_pairs.size(); ++i) {
        unordered_map<int, vector<int> >::const_iterator pair_i = sorted_i_ulist_pairs[i];
        if (show_progress) {
            fprintf(stderr, "\r%d of %d", idx++, items_num);
        }
        sim_list_buff.clear();

        //use_bitmap = true;
        use_bitmap = pair_i->second.size() > 50;
/** 
 *  由全部使用有序数组求交，改为 长用bitmap，短的遍历，时长由 30 分钟 提升到 12分钟（users num 100w+）
 *  // bitmapsize长度（users num）100万+的情况下，这个阈值选取0（即全部使用bitmap），50和100，时长都差不多。但是还是保留这个逻辑，单user_list长度达到千万时，这里根据阈值做区分对待应该还是有必要
 */
        if (use_bitmap) {
            for (vector<int>::const_iterator iter_pair_i = pair_i->second.begin(); iter_pair_i != pair_i->second.end(); ++iter_pair_i) {
                user_bm.Set(*iter_pair_i);
            }
        }

        for (int j = 0; j < i; ++j) {
            unordered_map<int, vector<int> >::const_iterator pair_j = sorted_i_ulist_pairs[j];
            users_intersection_buffer.clear();
            // 交互过item_i, item_j的user_list
            if (use_bitmap) {
                for (vector<int>::const_iterator iter_pair_j = pair_j->second.begin(); iter_pair_j != pair_j->second.end(); ++iter_pair_j) {
                    if (user_bm.Existed(*iter_pair_j)) {
                        users_intersection_buffer.push_back(*iter_pair_j);
                    }
                }
            } else {
                set_intersection(pair_i->second.begin(), pair_i->second.end(), pair_j->second.begin(), pair_j->second.end(), back_inserter(users_intersection_buffer));
            }

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

        sim_matrix[pair_i->first] = sim_list_buff; // TODO 这里可以先push_back一个空的，然后swap
        for (auto & p : sim_list_buff) {
            sim_matrix[p.first].push_back(make_pair(pair_i->first, p.second));
        }
        if (use_bitmap) {
            for (vector<int>::const_iterator iter_pair_i = pair_i->second.begin(); iter_pair_i != pair_i->second.end(); ++iter_pair_i) {
                user_bm.ResetRoughly(*iter_pair_i);
            }
        }
    }

    for (auto & p : sim_matrix) {
        vector<pair<int, float> > & sim_list = p.second;
        int sim_list_len = p.second.size();
        if (sim_list_len > 0) {
            // 普遍较长的话，这里可以make_heap 建一个长度为max_sim_list_len的堆，遍历一次得到topK就行
            sort(sim_list.begin(), sim_list.end(), compare_pairs);

            cout << p.first << "\t" << sim_list[0].first << ":" << sim_list[0].second;


            if (sim_list_len > max_sim_list_len) sim_list_len = max_sim_list_len;

            sim_list_len_statis[sim_list_len] += 1;

            for (int i = 1; i < sim_list_len; i++) {
            
                cout << ',' << sim_list[i].first << ':' << sim_list[i].second;
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
