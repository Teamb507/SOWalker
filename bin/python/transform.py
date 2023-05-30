import argparse
from collections import defaultdict

parser = argparse.ArgumentParser(
    description='Processing the rmat graph')
parser.add_argument("--file", type=str, help="the dataset file path",
                    default="/dataset/rmat24/rmat24.txt")
parser.add_argument("--output", type=str, help="the dataset file path",
                    default="/dataset/rmat24/rmat24.txt.tf")

args = parser.parse_args()

graph = defaultdict(list)

lines = 0
nvert = 0
with open(args.file, "r", encoding="utf-8") as f:
    for line in f:
        lines += 1
        if(lines % 10000 == 0):
            print("read lines ", lines)

        src, dst = line.split()
        graph[int(src)].append(int(dst))
        nvert = max(nvert, int(src), int(dst))

lines = 0
with open(args.output, 'w', encoding="utf-8") as f:
    for v in range(nvert + 1):
        if v in graph:
            for adj in graph[v]:
                line = str(v) + " " + str(adj) + "\n"
                f.write(line)

        lines += 1
        if(lines % 10000 == 0):
            print("write vertices ", lines)
