from operator import mod
import matplotlib.pyplot as plt
from matplotlib.pyplot import MultipleLocator
import numpy as np

config = {
    "font.family":'serif',
    "font.size": 14,
    "mathtext.fontset":'stix',
    "font.serif": ['SimSun'],
}
plt.rcParams.update(config)

walks = [7762, 122558, 11308, 4952, 6597, 50171, 156068, 24166, 36173, 101039, 86888, 19200, 116746, 143300, 31094, 55287, 18145]
x = np.arange(len(walks))
y = np.array(walks)
x_major_locator = MultipleLocator(1)
ax = plt.gca()
ax.xaxis.set_major_locator(x_major_locator)

plt.plot(x, y, 'o-', color='black')
plt.ylim(0, 160000)
plt.xlabel("子图分区")
plt.ylabel("分区中随机游走数目")
plt.grid(True, axis='y', linestyle=':')
plt.show()