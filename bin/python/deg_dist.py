import argparse
from collections import defaultdict
from matplotlib import markers
import matplotlib
import numpy as np
import matplotlib.pyplot as plt

config = {
    "font.family": 'serif',
    "font.size": 14,
    "mathtext.fontset": 'stix',
    "font.serif": ['SimSun'],
    "axes.unicode_minus": False,
}
plt.rcParams.update(config)

parser = argparse.ArgumentParser(
    description='Process skewed degree distribution')
parser.add_argument("--file", type=str, help="the dataset file path",
                    default="/home/hsc/twitter/twitter_rv.net")

args = parser.parse_args()

out_degrees = defaultdict(int)
in_degrees = defaultdict(int)

lines = 0

with open(args.file, "r", encoding="utf-8") as f:
    for line in f:
        src, dst = line.split()
        out_degrees[src] += 1
        in_degrees[dst] += 1
        lines += 1

        if lines % 10000 == 0:
            print("readlines : ", lines)

in_degree_cnts = defaultdict(int)
out_degree_cnts = defaultdict(int)

for key in in_degrees.keys():
    in_degree_cnts[in_degrees[key]] += 1

for key in out_degrees.keys():
    out_degree_cnts[out_degrees[key]] += 1

in_keys = in_degree_cnts.keys()
in_values = in_degree_cnts.values()

out_keys = out_degree_cnts.keys()
out_values = out_degree_cnts.values()


plt.figure()
ax1 = plt.subplot(121)
ax1.scatter(in_keys, in_values, color='black')
# ax1.set_xlim(left=0)
# ax1.set_ylim(bottom=0)
ax1.set_xlim(1, 10**6)
ax1.set_xscale('log')
ax1.set_yscale('log')
ax1.xaxis.set_major_locator(matplotlib.ticker.LogLocator(base=10, numticks=8))
ax1.set_xlabel("顶点入度大小")
ax1.set_ylabel("顶点数目")


ax2 = plt.subplot(122)
ax2.scatter(out_keys, out_values, color='black')
ax2.yaxis.tick_right()
# ax2.set_xlim(left=0)
# ax2.set_ylim(bottom=0)
ax2.set_xlim(1, 10**6)
ax2.yaxis.set_label_position("right")
ax2.set_xscale('log')
ax2.set_yscale('log')
ax2.xaxis.set_major_locator(matplotlib.ticker.LogLocator(base=10, numticks=8))
ax2.set_xlabel("顶点出度大小")
ax2.set_ylabel("顶点数目")


plt.show()
