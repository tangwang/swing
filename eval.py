#!/home/SanJunipero/anaconda3/bin/python
# -*- coding:UTF-8 -*-
import os,sys,json,re,time
import numpy as np
import pandas as pd
from itertools import combinations
import logging
import traceback
import cgitb
from argparse import ArgumentParser

sim_index = {}

max_fea = 20 #最多用x个历史交互id去召回
max_recall_len = 1200

def para_define(parser):
    parser.add_argument('-s', '--sim_index', type=str, default='')

def parse_sim_item_pair(x):
    x = x.split(':')
    return (int(x[0]), float(x[1]))

def parse_session_item_pair(x):
    x = x.split(':')
    return (int(x[0][1:-1]), float(x[1]))

def run_eval(FLAGS):
    with open(FLAGS.sim_index) as f:
        for line in f:
            segs = line.rstrip().split('\t')
            if len(segs) != 2:
                continue
            k, vlist = segs
            sim_index[int(k)] = [parse_sim_item_pair(x) for x in vlist.split(',')]
    
    statis = []
    for line in sys.stdin:
        line = line.strip()
        segs = line.split('\t')    
        uid = segs[0]
        session = segs[1][1:-1]
        if not session:
            continue
        session_list = [parse_session_item_pair(x) for x in session.split(',')]
    
        score_list = {}
        for item_id, wei in session_list[1:1+max_fea]:
            for sim_item_id, sim_value in sim_index.get(item_id, []):
                score_list.setdefault(sim_item_id, 0.0)
                score_list[sim_item_id] += wei*sim_value
        score_list.items()
        sorted_score_list = sorted(score_list.items(), key = lambda k:k[1], reverse=True)[:max_recall_len]
        
        target_item_id = session_list[0][0]
        hit_pos = -1
        for idx, (k, v) in enumerate(sorted_score_list):
            if target_item_id == k:
                hit_pos = idx
                break

        if hit_pos == -1 or hit_pos > max_recall_len:
            hit_pos =  max_recall_len
        info = (1, hit_pos, len(sorted_score_list),
            int(hit_pos < 25),
            int(hit_pos < 50),
            int(hit_pos < 100),
            int(hit_pos < 200),
            int(hit_pos < 400),
            int(hit_pos < 800),
            int(hit_pos < max_recall_len),
        )
        statis.append(info)
    statis = np.array(statis)

    desc = '''(1, hit_pos, len(sorted_score_list),
            int(hit_pos != -1 and hit_pos < 25),
            int(hit_pos != -1 and hit_pos < 50),
            int(hit_pos != -1 and hit_pos < 100),
            int(hit_pos != -1 and hit_pos < 200),
            int(hit_pos != -1 and hit_pos < 400),
            int(hit_pos != -1 and hit_pos < 800),
            int(hit_pos != -1),
        )'''
    print(desc)
 
    np.set_printoptions(suppress=True)
    print(FLAGS.sim_index, 'mean', '\t'.join([str(x) for x in statis.mean(axis=0)]), sep='\t')
    print(FLAGS.sim_index, 'sum', '\t'.join([str(x) for x in statis.sum(axis=0)]), sep='\t')



def main():
    cgitb.enable(format='text')
    # op config
    parser = ArgumentParser()
    para_define(parser)

    FLAGS, unparsed = parser.parse_known_args()
    print(FLAGS)

    run_eval(FLAGS)

if __name__ == "__main__":
    main()
