import matplotlib.pyplot as plt
import numpy as np
from matplotlib.pyplot import MultipleLocator

# plt.style.use(['science', 'ieee', 'no-latex', 'cjk-sc-font'])
def auto_text(rects):
    for rect in rects:
        axes1.text(rect.get_x()+0.12, rect.get_height(), rect.get_height(), ha='center', va='bottom')

config = {
    "font.family": 'Times New Roman',
    "font.size": 20,
    "mathtext.fontset": 'stix',
    "font.serif": ['SimSun'],
}
plt.rcParams.update(config)

# width of the bars
bar_width = 0.25


# graphwalker_bar = [1, 1, 1]
# grasorw_bar = [6.97588784, 23.22501383, 1.158986686]
# sowalker_bar = [62.7271331, 37.64027311, 18.0512667]

graphwalker_bar = [1, 1, 1]
grasorw_bar = [7.0, 23.2, 1.2]
sowalker_bar = [62.7, 37.6, 18.1]

dataset = ["$\mathrm{TW}$", "$\mathrm{FR}$", "$\mathrm{UK}$"]

x1 = np.arange(len(graphwalker_bar))
x2 = [x + bar_width for x in x1]
x3 = [x + 2 * bar_width for x in x1]

fig, axes1 = plt.subplots(figsize=(5.5, 5))
# axes1 = brokenaxes(ylims=((0, 40), (60, 65)), hspace=.03, despine=False)
lns1 = axes1.bar(x1, graphwalker_bar, color='white', width=bar_width,
                 label=r"$\mathrm{GraphWalker}$", edgecolor='black')
lns2 = axes1.bar(x2, grasorw_bar, color='grey', width=bar_width,
                 label=r"$\mathrm{GraSorw}$", edgecolor='black')
lns3 = axes1.bar(x3, sowalker_bar, color='black', width=bar_width,
                 label=r"$\mathrm{SOWalker}$", edgecolor='black')


axes1.set_xticks(
    [r + bar_width for r in range(len(sowalker_bar))], dataset)
axes1.ticklabel_format(axis='y', style='plain',
                       scilimits=(0, 0), useMathText=True) # stlye = 'sci'
axes1.set_ylabel("Normalized walk updating rate")
auto_text(lns1)
auto_text(lns2)
auto_text(lns3)
y_major_locator = MultipleLocator(20)
axes1.yaxis.set_major_locator(y_major_locator)
# axes1.set_yscale("log")
# axes1.grid(True, axis='y', linestyle=':')
axes1.set_ylim(0, 70)

axes1.legend(handles=[lns1, lns2, lns3], prop={'size': 20})
plt.savefig("Fig 7(b).png", dpi=400, bbox_inches="tight")
plt.show()
