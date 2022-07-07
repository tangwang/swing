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
#include <queue>
#include <numeric>
#include <fstream>
#include <thread>
#include "utils.h"
#include "BitMap.h"

using namespace std;

class Config {
public:
    Config() {

        user_sessions_num = 2000000;
        items_num = 160000;

        max_sim_list_len = 300;
        max_session_list_len = 100;

        threshold1 = 0.5;
        threshold2 = 0.5;
        alpha = 0.5;
        thread_num = 20;
        show_progress = 0;
        output_path = "result";
    }
       
    int load(int argc,char *argv[]) {
        if (argc < 7) {
            cout << "usage " << argv[0] << " alpha threshold1 threshold2 thread_num output_path show_progress(0/1) " << endl;
            return -1;
        }
    
        alpha = atof(argv[1]);
        threshold1 = atof(argv[2]);
        threshold2 = atof(argv[3]);
    
        thread_num = atoi(argv[4]);
        output_path = argv[5];
        show_progress = atoi(argv[6]);
    
        cout << currentTimetoStr() << " start... " << endl;
        cout << " threshold1 " << threshold1 << endl;
        cout << " threshold2 " << threshold2 << endl;
        cout << " alpha " << alpha << endl;
        return 0;
    }

public:
    int user_sessions_num; 
    int items_num;

    int max_sim_list_len; // 输出相似itemlist 最大长度
    int max_session_list_len; // 输入的 用户行为列表，截断长度（按权重排序阶段）
    float threshold1;
    float threshold2;
    float alpha;
    float thread_num;
    int show_progress;
    string output_path;
};

/**
 *
 * read data from stdin
 * format:
 * 输入的itemlist必须是按照权重排序的
 *
 * {"111":3.9332,"222":0.0382,"333":0.0376}
 * {"444":13.2136,"555":2.1438,"666":1.3443,"777":0.6775}
 * {"888":22.0632,"999":0.0016}
 *
 * parm : 
 * config
 * groups : index of user_id -> items
 * i2u_map : index of item -> users
 */
int load_data(const Config & config,
         vector< pair<vector<int> , vector<int> > > & groups,
         unordered_map<int, pair<vector<int>, vector<int> > > & i2u_map) {

    string line_buff;

    const string delimiters(",");

    vector<string> field_segs;
    // 每个元素是一个user的两个itemlist，first是交互强度大于threshold1的itemList，后者是强度大于threshold2的itemList
    pair<vector<int> , vector<int>  > itemlist_pair;
    

    pair<int, pair<vector<int> , vector<int>  >  > pair_entry;
    pair<unordered_map<int, pair<vector<int> , vector<int> > >::iterator, bool> ins_i2u_ret;

    while (getline(cin, line_buff)) {
         //格式是一个json，所以要把开头和结尾的括号去掉
        line_buff.erase(0,line_buff.find_first_not_of("{"));
        line_buff.erase(line_buff.find_last_not_of("}") + 1);
        //cout << line_buff << " !!!" << endl;
        field_segs.clear();
        split(field_segs, line_buff, delimiters);
        if (field_segs.size() < config.max_session_list_len) {
            field_segs.resize(config.max_session_list_len);
        }

        // field_segs是按权重有序的，进行截断

        for (size_t i = 0; i < field_segs.size(); i++) {
            const char * seg_pos = strchr(field_segs[i].c_str(), ':') ;
            if (seg_pos == NULL || (seg_pos - field_segs[i].c_str() >= field_segs[i].length())) break;

            float value = atof(seg_pos + 1);
            if (value < config.threshold1 && value < config.threshold2) break;

            // 开头有一个双引号
            int item_id = atoi(field_segs[i].c_str() + 1);
            if (value > config.threshold1) {
                itemlist_pair.first.push_back(item_id);
            }
            if (value > config.threshold2) {
                itemlist_pair.second.push_back(item_id);
            }
        }

        // 左侧必须有2个item，右侧必须有1个item，此时该用户才有可能给(item_i, item_j) 打分
        if (!(itemlist_pair.first.size() > 1 && itemlist_pair.second.size() > 0)) {
            itemlist_pair.first.clear();
            itemlist_pair.second.clear();
            continue;
        }
        // 排序
        sort(itemlist_pair.first.begin(), itemlist_pair.first.end());
        sort(itemlist_pair.second.begin(), itemlist_pair.second.end());
        
        // 合入i2u索引
        int idx = groups.size(); //待插入的index
        for (auto item_id : itemlist_pair.first) {
            pair_entry.first = item_id;
            ins_i2u_ret = i2u_map.insert(pair_entry);
            ins_i2u_ret.first->second.first.push_back(idx);
        }
        for (auto item_id : itemlist_pair.second) {
            pair_entry.first = item_id;
            ins_i2u_ret = i2u_map.insert(pair_entry);
            ins_i2u_ret.first->second.second.push_back(idx);
        }

        // 插入 u -> item_list索引
        groups.resize(groups.size()+1);
        groups.back().first.swap(itemlist_pair.first);
        groups.back().second.swap(itemlist_pair.second);

    }

    cout << currentTimetoStr() << " items num: " << i2u_map.size() << endl;
    cout << currentTimetoStr() << " users num: " << groups.size() << endl;
    cout << currentTimetoStr() <<  " sort.." << endl;

    for (auto iter : i2u_map) {
        sort(iter.second.first.begin(), iter.second.first.end());
        sort(iter.second.second.begin(), iter.second.second.end());
    }
    cout << currentTimetoStr() << " sort finished" << endl;
    return 0;
 
}


struct TaskOutput {
    int id;
    string output_path;
    vector<int> sim_list_len_statis;
};


/*
 * input parm:
 * groups : u -> i index 
 * i2u_map : i -> u index 
 * output_path : path of write sim matrix
 *
 * output param:
 * out
 *
 */
int calc_sim_matrix(const Config & config,
         const vector< pair<vector<int> , vector<int> > > & groups,
         const unordered_map<int, pair<vector<int>, vector<int> > > & i2u_map,
         TaskOutput & out,
         int task_id, int total_tasks
) {

    int users_num = groups.size();
    int items_num = i2u_map.size();
    if (items_num < 2) return -1;

    ofstream out_file(out.output_path);
    if (out_file.fail()) {
        cerr << currentTimetoStr() << " create out_file err: " << out.output_path << endl;
        return -1;
    }

    vector<int> users_intersection_buffer;
    vector<int> items_intersection_buffer;
    vector<pair<int, float> > sim_list_buff;
    users_intersection_buffer.reserve(2048);
    BitMap user_bm(users_num);
    bool use_bitmap;

    out.sim_list_len_statis.resize(config.max_sim_list_len+1);

    int idx = 0;
    for (auto & iter_i : i2u_map) {
        if ((idx++) % total_tasks != task_id) continue;

        const vector<int> & ulist_of_item_i = iter_i.second.first;
        if (config.show_progress) {
            fprintf(stdout, "\r%d of %d", idx++, items_num);
        }
        sim_list_buff.clear();

        //use_bitmap = true;
        use_bitmap = ulist_of_item_i.size() > 50;
/** 
 *  由全部使用有序数组求交，改为 长用bitmap，短的遍历，时长由 30 分钟 提升到 12分钟（users num 100w+）
 *  // bitmapsize长度（users num）100万+的情况下，这个阈值选取0（即全部使用bitmap），50和100，时长都差不多。但是还是保留这个逻辑，单user_list长度达到千万时，这里根据阈值做区分对待应该还是有必要
 */
        if (use_bitmap) {
            for (auto item_i : ulist_of_item_i) {
                user_bm.Set(item_i);
            }
        }

        for (auto & iter_j : i2u_map) {
            if (iter_j.first == iter_i.first) continue;

            const vector<int> & ulist_of_item_j = iter_j.second.second;
            users_intersection_buffer.clear();
            // 交互过item_i, item_j的user_list
            if (use_bitmap) {
                for (auto item_j : ulist_of_item_j) {
                    if (user_bm.Existed(item_j)) {
                        users_intersection_buffer.push_back(item_j);
                    }
                }
            } else {
                set_intersection(ulist_of_item_i.begin(), ulist_of_item_i.end(), ulist_of_item_j.begin(), ulist_of_item_j.end(), back_inserter(users_intersection_buffer));
            }

            if (users_intersection_buffer.size() < 2) continue;
            // user_i, user_j

            float sim_of_item_i_j = 0.0;
            // 遍历共同交互过(item_i, item_j)的user组合(user_i, user_j)
            for (vector<int>::const_iterator user_i = users_intersection_buffer.begin()+1;
                user_i != users_intersection_buffer.end();
                ++user_i) {

                //const vector<int> & item_list_of_user_i = groups[*user_i].second; // TODO 这里用first和second效果不同. first较泛，second较严较少
                const vector<int> & item_list_of_user_i = groups[*user_i].first; // TODO 这里用first和second效果不同. first较泛，second较严较少

                for (vector<int>::const_iterator user_j = users_intersection_buffer.begin();
                    user_j != user_i;
                    ++user_j) {

                    //const vector<int> & item_list_of_user_j = groups[*user_j].second;  // TODO 这里用first和second效果不同. first较泛，second较严较少
                    const vector<int> & item_list_of_user_j = groups[*user_j].first;  // TODO 这里用first和second效果不同. first较泛，second较严较少

                    items_intersection_buffer.clear();
                    // user_i, user_j交互过的items的交集
                    set_intersection(item_list_of_user_i.begin(), item_list_of_user_i.end(), item_list_of_user_j.begin(), item_list_of_user_j.end(), back_inserter(items_intersection_buffer));//求交集

                    sim_of_item_i_j += 1.0 / (config.alpha + items_intersection_buffer.size());
                }
            }
            sim_list_buff.push_back(make_pair(iter_j.first, sim_of_item_i_j));
        }

        if (use_bitmap) {
            for (auto uid : ulist_of_item_i) {
                user_bm.ResetRoughly(uid);
            }
        }

        int sim_list_len = sim_list_buff.size();
        if (sim_list_len > 0) {

            sort(sim_list_buff.begin(), sim_list_buff.end(), compare_pairs);

            out_file << iter_i.first << "\t" << sim_list_buff[0].first << ":" << sim_list_buff[0].second;

            if (sim_list_len > config.max_sim_list_len) sim_list_len = config.max_sim_list_len;

            out.sim_list_len_statis[sim_list_len] += 1;

            for (int i = 1; i < sim_list_len; i++) {

                out_file << ',' << sim_list_buff[i].first << ':' << sim_list_buff[i].second;
            }
            out_file << endl; 
        }

    }

    out_file.close();
    return 0;
} 

void printSimMatrixStatisInfo(string task_name, const vector<int> & sim_list_len_statis) {
    // staits info of sim matrix
    int sum_groups = accumulate(sim_list_len_statis.begin(), sim_list_len_statis.end(), (int)0);
    cout << currentTimetoStr() <<  " ========== TASK STATIS INFO [" << task_name << "]==========" << endl;
    cout << currentTimetoStr() <<  " write sim matrix finished" << endl;
    cout << currentTimetoStr() <<  " print staits info of sim matrix... " << sim_list_len_statis.size() << endl;
    cout << currentTimetoStr() <<  " total keys: " << sum_groups  << endl;

    int accumulate = 0;
    for (int i = sim_list_len_statis.size()-1; i >= 0; i--) {
        accumulate += sim_list_len_statis[i];
        if (i % 20 == 0) {
            // 注意 为防止输出太多，间隔20输出一行，所以num与上一行的累加不会等于accumulate
            fprintf(stdout, "simlist_len %4d, num %4d, accumulate %6d accumulated_rate %5.2f%\%\n", (int)i, sim_list_len_statis[i], accumulate, 100.0*accumulate/sum_groups);
        }
    }
}
 
int main(int argc,char *argv[]) {

    Config config;
    int ret = config.load(argc, argv);
    if (ret < 0) {
        cerr << currentTimetoStr() << " load_config err: " << ret << endl;
        return ret;
    }

    cout << currentTimetoStr() << " start load raw user_session data ... " << endl;

    vector< pair<vector<int> , vector<int> > > groups;
    groups.reserve(config.user_sessions_num);

    unordered_map<int, pair<vector<int>, vector<int> > > i2u_map;
    i2u_map.reserve(config.items_num);

    ret = load_data(config, groups, i2u_map);
    if (ret < 0) {
        cerr << currentTimetoStr() << " load_data err: " << ret << endl;
        return ret;
    }
    cout << currentTimetoStr() << " load raw user_session data finished. " << endl;

    vector<TaskOutput> outs;
    outs.resize(config.thread_num);

    vector<thread> threads;
    char out_path[256];
    for (int task_id = 0; task_id < config.thread_num; task_id++) {
        outs[task_id].id = task_id;

        snprintf(out_path, sizeof(out_path), "%s/sim_matrx.%0.1f_%0.3f_%0.3f.%d", config.output_path.c_str(), config.alpha, config.threshold1, config.threshold2, task_id);
        outs[task_id].output_path = out_path;
        threads.push_back(thread(calc_sim_matrix, std::cref(config), std::cref(groups), std::cref(i2u_map), std::ref(outs[task_id]), task_id, config.thread_num));
    }

    // wait all tasks
    cout << endl;
    cout << currentTimetoStr() << " wait sim_calc threads ... " << endl;
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    cout << currentTimetoStr() << " all sim_calc tasks finished" << endl;

    // merge outputs
    TaskOutput merged_output;
    vector<int> & sim_list_len_statis = merged_output.sim_list_len_statis;
    for (auto & out_task_i : outs) {
        string task_name = std::to_string(out_task_i.id) + " " + out_task_i.output_path;
        printSimMatrixStatisInfo(task_name, out_task_i.sim_list_len_statis);

        vector<int> & list_i = out_task_i.sim_list_len_statis;
        if (sim_list_len_statis.size() < list_i.size()) {
            sim_list_len_statis.resize(list_i.size());
        }
        for (size_t j = 0; j < list_i.size(); j++) {
            sim_list_len_statis[j] += list_i[j];
        }
    }

    printSimMatrixStatisInfo("Merged", sim_list_len_statis);

    return 0;
}
