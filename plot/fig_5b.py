import matplotlib.pyplot as plt
import numpy as np

# plt.style.use(['science', 'ieee', 'no-latex', 'cjk-sc-font'])

config = {
    "font.family": 'Times New Roman',
    "font.size": 20,
    "mathtext.fontset": 'stix',
    "font.serif": ['SimSun'],
}
plt.rcParams.update(config)

# width of the bars
bar_width = 0.25

sowalker_path ="/log/sowalker_pagerank_log"
graphwalker_path="/log/graphwalker_pagerank_log"
grasorw_path="/log/grasorw_pagerank_log"

graphwalker_bar = []
grasorw_bar = []
sowalker_bar = []
with open(sowalker_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find(".run_app=")!=-1):
            sowalker_bar.append(float(dataline[dataline.find("=")+1:]))
with open(graphwalker_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find(".run_app=")!=-1):
            graphwalker_bar.append(float(dataline[dataline.find("=")+1:]))
with open(grasorw_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find(".run_app=")!=-1):
            grasorw_bar.append(float(dataline[dataline.find("=")+1:]))
 for i in range(5):
    sowalker_bar[i]=( sowalker_bar[i]/graphwalker_bar[i])
    grasorw_bar[i]=(grasorw_bar[i]/graphwalker_bar[i])
    graphwalker_bar[i]=1

# ratio = [0.111957404, 0.458702192, 0.359379036, 0.459004998, 0.58698386, 0.692784113]
dataset = ["$\mathrm{TW}$", "$\mathrm{FR}$", "$\mathrm{UK}$", "$\mathrm{K30}$", "$\mathrm{CW}$"]

x1 = np.arange(len(graphwalker_bar))
x2 = [x + bar_width for x in x1]
x3 = [x + 2 * bar_width for x in x1]

fig, axes1 = plt.subplots(figsize=(5.5, 5))
lns1 = axes1.bar(x1, graphwalker_bar, color='white', width=bar_width,
                 label=r"$\mathrm{GraphWalker}$", edgecolor='black')
lns2 = axes1.bar(x2, grasorw_bar, color='grey', width=bar_width,
                 label=r"$\mathrm{GraSorw}$", edgecolor='black')
lns3 = axes1.bar(x3, sowalker_bar, color='black', width=bar_width,
                 label=r"$\mathrm{SOWalker}$", edgecolor='black')


axes1.set_xticks(
    [r + bar_width for r in range(len(sowalker_bar))], dataset)
axes1.ticklabel_format(axis='y', style='sci',
                       scilimits=(0, 0), useMathText=True)
axes1.set_ylabel("Normalized execution time")
# axes1.set_yscale("log")
axes1.grid(True, axis='y', linestyle=':')
# axes1.set_ylim(30, 10000000)

axes1.legend(handles=[lns1, lns2, lns3], prop={'size': 17}, ncol=2, bbox_to_anchor=(0.5, 1), loc=9, borderaxespad=-3.5)
plt.savefig("Fig 5(b).png", dpi=300, bbox_inches="tight")
plt.show()
