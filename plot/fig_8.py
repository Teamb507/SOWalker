"""
@author hsc
@application: node2vec
@dataset: all dataset
@description: total execution time
"""

import matplotlib.pyplot as plt
import numpy as np

# plt.style.use(['science', 'ieee', 'no-latex', 'cjk-sc-font'])

config = {
    "font.family": 'Times New Roman',
    "font.size": 19,
    "mathtext.fontset": 'stix',
    "font.serif": ['SimSun'],
}
plt.rcParams.update(config)

# width of the bars
bar_width = 0.25
log_path="/log/blocksize_log"
index=0
twitter_sowalker =[]
twitter_graphwalker =[]
friendster_sowalker = []
friendster_graphwalker =[]
ukunion_sowalker = []
ukunion_graphwalker = []
with open(sowalker_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find(".run_app=")!=-1):
            if(index==0):
                twitter_sowalker.append(float(dataline[dataline.find("=")+1:]))
            elif (index==1):
               friendster_sowalker.append(float(dataline[dataline.find("=")+1:]))
            elif (index==2):
               ukunion_sowalker.append(float(dataline[dataline.find("=")+1:]))
            elif (index==3):
                twitter_graphwalker.append(float(dataline[dataline.find("=")+1:]))
            elif (index==4):
                friendster_graphwalker.append(float(dataline[dataline.find("=")+1:]))
            else:
                ukunion_graphwalker.append(float(dataline[dataline.find("=")+1:]))
            index=index+1
            if(index==6):
	index=0

dataset = ["$\mathrm{128MB}$", "$\mathrm{256MB}$", "$\mathrm{512MB}$", "$\mathrm{1GB}$", "$\mathrm{2GB}$"]

x1 = np.arange(len(twitter_sowalker))
x2 = [x + bar_width for x in x1]

fig, axes1 = plt.subplots()
lns1 = axes1.plot(x1, twitter_sowalker, 'black', marker='x', markersize=6,
                  label=r"Twitter")
lns2 = axes1.plot(x1, friendster_sowalker, 'blue', marker='+', markersize=8,
                  label=r"Friendster")
lns3 = axes1.plot(x1, ukunion_sowalker, 'red', marker='o', markersize=4,
                  label=r"UK-Union")
lns4 = axes1.plot(x1, twitter_graphwalker, 'black', linestyle='--', label=r"GraphWalker")
lns5 = axes1.plot(x1, twitter_sowalker, 'black', label=r"SOWalker")

axes1.plot(x1, twitter_sowalker, 'black', marker='x', markersize=6,
                  label=r"SOWalker")
axes1.plot(x1, twitter_graphwalker, 'black', linestyle='--', marker='x', markersize=6,
                  label=r"GraphWalker")
axes1.plot(x1, friendster_sowalker, 'blue', marker='+', markersize=6,
                  label=r"SOWalker")
axes1.plot(x1, friendster_graphwalker, 'blue', linestyle='--', marker='+', markersize=6,
                  label=r"GraphWalker")
axes1.plot(x1, ukunion_sowalker, 'red', marker='o', markersize=6,
                  label=r"SOWalker")
axes1.plot(x1, ukunion_graphwalker, 'red', linestyle='--', marker='o', markersize=6,
                  label=r"GraphWalker")

axes1.set_xticks(
    [r for r in range(len(twitter_sowalker))], dataset)
axes1.ticklabel_format(axis='y', style='sci',
                       scilimits=(0, 0), useMathText=True)
axes1.set_ylabel("Execution time (s)")
# axes1.set_xlabel("数据集")
axes1.set_yscale("log")
axes1.grid(True, axis='y', linestyle=':')
axes1.set_ylim(1000, 30000)



axes1.legend(handles=[lns1[0], lns4[0], lns2[0], lns5[0], lns3[0]], prop={'size': 17}, ncol=3, bbox_to_anchor=(0.5, 1.05), loc=9, borderaxespad=-2.5)
plt.savefig("Fig 8.png", dpi=300, bbox_inches="tight")
plt.show()

