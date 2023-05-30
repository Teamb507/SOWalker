from cProfile import label
from logging import handlers
from matplotlib import markers
import matplotlib.pyplot as plt
import numpy as np

plt.style.use(['science', 'ieee', 'no-latex', 'cjk-sc-font'])

config = {
    "font.family": 'serif',
    "font.size": 6,
    "mathtext.fontset": 'stix',
    "font.serif": ['SimSun'],
}
plt.rcParams.update(config)

# width of the bars
bar_width = 0.15

sowalker_bar = [33.3137, 560.56, 806.326, 1759.92]
graphwalker_bar = [32.4159, 874.026, 1490.45, 5728.61]
ratio = [0.973050127, 1.560985443, 1.848445914, 3.255040002]


dataset = ["$\mathrm{SL}$", "$\mathrm{TW}$", "$\mathrm{CF}$", "$\mathrm{UK}$"]

x1 = np.arange(len(sowalker_bar))
x2 = [x + bar_width for x in x1]

fig, axes1 = plt.subplots()

# plt.bar(x1, sowalker_bar, width=bar_width, color="#8E8BFE", label=r"$\mathrm{SOWalker}$")
# plt.bar(x2, graphwalker_bar, width=bar_width, color="#FEA3A2", label=r"$\mathrm{GraphWalker}$")
lns1 = axes1.bar(x1, sowalker_bar, width=bar_width,
                 label=r"$\mathrm{SOWalker}$")
lns2 = axes1.bar(x2, graphwalker_bar, width=bar_width,
                 label=r"$\mathrm{GraphWalker}$", edgecolor='black', hatch='//')

axes1.set_xticks(
    [r + bar_width / 2 for r in range(len(sowalker_bar))], dataset)
axes1.ticklabel_format(axis='y', style='sci',
                       scilimits=(0, 0), useMathText=True)
axes1.set_ylabel("时间/s")
# axes1.legend()

axes2 = axes1.twinx()
lns3 = axes2.plot(x1, ratio, 'g', marker='o', markersize=2,
                  label=r"加速比$\mathrm{\beta}$")
axes2.set_ylabel("加速比")
axes2.set_ylim(0, 3.5)
# axes2.legend(loc='best')

axes1.legend(handles=[lns1, lns2, lns3[0]])


plt.grid(True, axis='y', linestyle='--')

# plt.autoscale(tight=True)
plt.savefig("time.png", dpi=300)
