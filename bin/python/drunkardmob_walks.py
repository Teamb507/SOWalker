#
# author : 2022/2/8
# This file constributes to deal with the drunkardmob log file

import argparse

parser = argparse.ArgumentParser(description='Process drunkardmob log file')
parser.add_argument("--file", type=str, help="the drunkardmob log file path", default="/home/hsc/graphchi-java/tmp.log")
parser.add_argument("--interval", type=str, help="the drunkardmob vertex interval file path", default="/home/hsc/dataset/livejournal/soc-LiveJournal1.txt.16.intervalsjava")
parser.add_argument("--iter", type=int, help="the iteration", default=3)

args = parser.parse_args()

def get_interval_index(vertex, intervals) :
    ans = len(intervals)
    for idx in range(len(intervals)):
        if(vertex < intervals[idx]):
            ans = idx
            break
    return ans

intervals = []
with open(args.interval, "r", encoding='utf-8') as file:
    for line in file:
        intervals.append(int(line))

cur_iter = 0
walks_count = [0] * len(intervals)
with open(args.file, "r", encoding='utf-8') as file:
    for line in file:
        if "iteration" in line:
            cur_iter = int(line.split()[-1])
        else:
            if cur_iter != args.iter:
                continue
            else:
                words = line.split()
                vertex, num = int(words[-4]), int(words[-1])
                walks_count[get_interval_index(vertex, intervals)] += num

print(' '.join(map(str, walks_count)))
