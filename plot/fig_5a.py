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


graphwalker_bar = [1, 1, 1, 1, 1]
grasorw_bar = [0.846153846, 0.33413102, 0.988767989, 0.684824624, 0.555555488]
sowalker_bar = [0.738974359, 0.311845648, 0.374341874, 0.120143049, 0.178596491]


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
# axes1.set_ylim(0, 1.2)

axes1.legend(handles=[lns1, lns2, lns3], prop={'size': 17}, ncol=2, bbox_to_anchor=(0.5, 1), loc=9, borderaxespad=-3.5)
plt.savefig("Fig 5(a).png", dpi=300, bbox_inches="tight")
plt.show()
