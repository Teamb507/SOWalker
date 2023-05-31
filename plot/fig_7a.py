import matplotlib.pyplot as plt
import numpy as np

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


graphwalker_bar = []
grasorw_bar = []
sowalker_bar = []

input_path ="log/tw_sowalker_log"
total_steps=0
block_id=0
block_edges=0
block_id_bar=[]
block_edges_bar=[]
steps_bar=[]
ute_bar=[]
pre_steps=0
current_steps=0
io_number=0
with open(input_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find("current_stepsis")!=-1):
            # print(io_number)
            total_steps=int(dataline[dataline.find(":")+1:])
            current_steps=total_steps-pre_steps
            pre_steps=current_steps
            if(io_number!=0 and current_steps!=0):
                for i in range(io_number):
                    steps_bar.append(current_steps/io_number)
            io_number = 0
        if(dataline.find("currentloadblockis")!=-1):
            io_number += 1
            block_id=int(dataline[dataline.find("kis")+3:dataline.find(";")])
            block_id_bar.append(block_id)
            block_edges=int(dataline[dataline.find(":")+1:])
            block_edges_bar.append(block_edges)
for i in range(len(steps_bar)):
    ute_bar.append(float(steps_bar/block_edges_bar))
sowalker_bar.append(sum(ute_bar)/len(ute_bar))

input_path ="log/tw_sowalker_log"
total_steps=0
block_id=0
block_edges=0
block_id_bar=[]
block_edges_bar=[]
steps_bar=[]
ute_bar=[]
pre_steps=0
current_steps=0
io_number=0
with open(input_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find("current_stepsis")!=-1):
            # print(io_number)
            total_steps=int(dataline[dataline.find(":")+1:])
            current_steps=total_steps-pre_steps
            pre_steps=current_steps
            if(io_number!=0 and current_steps!=0):
                for i in range(io_number):
                    steps_bar.append(current_steps/io_number)
            io_number = 0
        if(dataline.find("currentloadblockis")!=-1):
            io_number += 1
            block_id=int(dataline[dataline.find("kis")+3:dataline.find(";")])
            block_id_bar.append(block_id)
            block_edges=int(dataline[dataline.find(":")+1:])
            block_edges_bar.append(block_edges)
for i in range(len(steps_bar)):
    ute_bar.append(float(steps_bar/block_edges_bar))
sowalker_bar.append(sum(ute_bar)/len(ute_bar))

input_path ="log/fr_sowalker_log"
total_steps=0
block_id=0
block_edges=0
block_id_bar=[]
block_edges_bar=[]
steps_bar=[]
ute_bar=[]
pre_steps=0
current_steps=0
io_number=0
with open(input_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find("current_stepsis")!=-1):
            # print(io_number)
            total_steps=int(dataline[dataline.find(":")+1:])
            current_steps=total_steps-pre_steps
            pre_steps=current_steps
            if(io_number!=0 and current_steps!=0):
                for i in range(io_number):
                    steps_bar.append(current_steps/io_number)
            io_number = 0
        if(dataline.find("currentloadblockis")!=-1):
            io_number += 1
            block_id=int(dataline[dataline.find("kis")+3:dataline.find(";")])
            block_id_bar.append(block_id)
            block_edges=int(dataline[dataline.find(":")+1:])
            block_edges_bar.append(block_edges)
for i in range(len(steps_bar)):
    ute_bar.append(float(steps_bar/block_edges_bar))
sowalker_bar.append(sum(ute_bar)/len(ute_bar))

input_path ="log/uk_sowalker_log"
total_steps=0
block_id=0
block_edges=0
block_id_bar=[]
block_edges_bar=[]
steps_bar=[]
ute_bar=[]
pre_steps=0
current_steps=0
io_number=0
with open(input_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find("current_stepsis")!=-1):
            # print(io_number)
            total_steps=int(dataline[dataline.find(":")+1:])
            current_steps=total_steps-pre_steps
            pre_steps=current_steps
            if(io_number!=0 and current_steps!=0):
                for i in range(io_number):
                    steps_bar.append(current_steps/io_number)
            io_number = 0
        if(dataline.find("currentloadblockis")!=-1):
            io_number += 1
            block_id=int(dataline[dataline.find("kis")+3:dataline.find(";")])
            block_id_bar.append(block_id)
            block_edges=int(dataline[dataline.find(":")+1:])
            block_edges_bar.append(block_edges)
for i in range(len(steps_bar)):
    ute_bar.append(float(steps_bar/block_edges_bar))
sowalker_bar.append(sum(ute_bar)/len(ute_bar))

input_path ="log/tw_graphwalker_log"
total_steps=0
block_id=0
block_edges=0
block_id_bar=[]
block_edges_bar=[]
steps_bar=[]
ute_bar=[]
pre_steps=0
current_steps=0
io_number=0
with open(input_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find("current_stepsis")!=-1):
            # print(io_number)
            total_steps=int(dataline[dataline.find(":")+1:])
            current_steps=total_steps-pre_steps
            pre_steps=current_steps
            if(io_number!=0 and current_steps!=0):
                for i in range(io_number):
                    steps_bar.append(current_steps/io_number)
            io_number = 0
        if(dataline.find("currentloadblockis")!=-1):
            io_number += 1
            block_id=int(dataline[dataline.find("kis")+3:dataline.find(";")])
            block_id_bar.append(block_id)
            block_edges=int(dataline[dataline.find(":")+1:])
            block_edges_bar.append(block_edges)
for i in range(len(steps_bar)):
    ute_bar.append(float(steps_bar/block_edges_bar))
graphwalker_bar.append(sum(ute_bar)/len(ute_bar))

input_path ="log/fr_sowalker_log"
total_steps=0
block_id=0
block_edges=0
block_id_bar=[]
block_edges_bar=[]
steps_bar=[]
ute_bar=[]
pre_steps=0
current_steps=0
io_number=0
with open(input_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find("current_stepsis")!=-1):
            # print(io_number)
            total_steps=int(dataline[dataline.find(":")+1:])
            current_steps=total_steps-pre_steps
            pre_steps=current_steps
            if(io_number!=0 and current_steps!=0):
                for i in range(io_number):
                    steps_bar.append(current_steps/io_number)
            io_number = 0
        if(dataline.find("currentloadblockis")!=-1):
            io_number += 1
            block_id=int(dataline[dataline.find("kis")+3:dataline.find(";")])
            block_id_bar.append(block_id)
            block_edges=int(dataline[dataline.find(":")+1:])
            block_edges_bar.append(block_edges)
for i in range(len(steps_bar)):
    ute_bar.append(float(steps_bar/block_edges_bar))
graphwalker_bar.append(sum(ute_bar)/len(ute_bar))

input_path ="log/uk_sowalker_log"
total_steps=0
block_id=0
block_edges=0
block_id_bar=[]
block_edges_bar=[]
steps_bar=[]
ute_bar=[]
pre_steps=0
current_steps=0
io_number=0
with open(input_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find("current_stepsis")!=-1):
            # print(io_number)
            total_steps=int(dataline[dataline.find(":")+1:])
            current_steps=total_steps-pre_steps
            pre_steps=current_steps
            if(io_number!=0 and current_steps!=0):
                for i in range(io_number):
                    steps_bar.append(current_steps/io_number)
            io_number = 0
        if(dataline.find("currentloadblockis")!=-1):
            io_number += 1
            block_id=int(dataline[dataline.find("kis")+3:dataline.find(";")])
            block_id_bar.append(block_id)
            block_edges=int(dataline[dataline.find(":")+1:])
            block_edges_bar.append(block_edges)
for i in range(len(steps_bar)):
    ute_bar.append(float(steps_bar/block_edges_bar))
graphwalker_bar.append(sum(ute_bar)/len(ute_bar))


input_path ="log/tw_grasorw_log"
total_steps=0
block_id=0
block_edges=0
block_id_bar=[]
block_edges_bar=[]
steps_bar=[]
ute_bar=[]
pre_steps=0
current_steps=0
io_number=0
with open(input_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find("current_stepsis")!=-1):
            # print(io_number)
            total_steps=int(dataline[dataline.find(":")+1:])
            current_steps=total_steps-pre_steps
            pre_steps=current_steps
            if(io_number!=0 and current_steps!=0):
                for i in range(io_number):
                    steps_bar.append(current_steps/io_number)
            io_number = 0
        if(dataline.find("currentloadblockis")!=-1):
            io_number += 1
            block_id=int(dataline[dataline.find("kis")+3:dataline.find(";")])
            block_id_bar.append(block_id)
            block_edges=int(dataline[dataline.find(":")+1:])
            block_edges_bar.append(block_edges)
for i in range(len(steps_bar)):
    ute_bar.append(float(steps_bar/block_edges_bar))
grasorw_bar.append(sum(ute_bar)/len(ute_bar))

input_path ="log/fr_sowalker_log"
total_steps=0
block_id=0
block_edges=0
block_id_bar=[]
block_edges_bar=[]
steps_bar=[]
ute_bar=[]
pre_steps=0
current_steps=0
io_number=0
with open(input_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find("current_stepsis")!=-1):
            # print(io_number)
            total_steps=int(dataline[dataline.find(":")+1:])
            current_steps=total_steps-pre_steps
            pre_steps=current_steps
            if(io_number!=0 and current_steps!=0):
                for i in range(io_number):
                    steps_bar.append(current_steps/io_number)
            io_number = 0
        if(dataline.find("currentloadblockis")!=-1):
            io_number += 1
            block_id=int(dataline[dataline.find("kis")+3:dataline.find(";")])
            block_id_bar.append(block_id)
            block_edges=int(dataline[dataline.find(":")+1:])
            block_edges_bar.append(block_edges)
for i in range(len(steps_bar)):
    ute_bar.append(float(steps_bar/block_edges_bar))
grasorw_bar.append(sum(ute_bar)/len(ute_bar))

input_path ="log/uk_sowalker_log"
total_steps=0
block_id=0
block_edges=0
block_id_bar=[]
block_edges_bar=[]
steps_bar=[]
ute_bar=[]
pre_steps=0
current_steps=0
io_number=0
with open(input_path, 'r', encoding='utf-8') as infile:
    for line in infile:
        data_line = line.strip("\n").split()
        dataline = ''.join(data_line)
        if(dataline.find("current_stepsis")!=-1):
            # print(io_number)
            total_steps=int(dataline[dataline.find(":")+1:])
            current_steps=total_steps-pre_steps
            pre_steps=current_steps
            if(io_number!=0 and current_steps!=0):
                for i in range(io_number):
                    steps_bar.append(current_steps/io_number)
            io_number = 0
        if(dataline.find("currentloadblockis")!=-1):
            io_number += 1
            block_id=int(dataline[dataline.find("kis")+3:dataline.find(";")])
            block_id_bar.append(block_id)
            block_edges=int(dataline[dataline.find(":")+1:])
            block_edges_bar.append(block_edges)
for i in range(len(steps_bar)):
    ute_bar.append(float(steps_bar/block_edges_bar))
grasorw_bar.append(sum(ute_bar)/len(ute_bar))

 for i in range(3):
    sowalker_bar[i]=( sowalker_bar[i]/graphwalker_bar[i])
    grasorw_bar[i]=(grasorw_bar[i]/graphwalker_bar[i])
    graphwalker_bar[i]=1
dataset = ["$\mathrm{TW}$", "$\mathrm{FR}$", "$\mathrm{UK}$"]

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
axes1.ticklabel_format(axis='y', style='plain',
                       scilimits=(0, 0), useMathText=True) # stlye = 'sci'
axes1.set_ylabel("Normalized I/O utilization")
auto_text(lns1)
auto_text(lns2)
auto_text(lns3)
# axes1.set_yscale("log")
# axes1.grid(True, axis='y', linestyle=':')
axes1.set_ylim(0, 38)

axes1.legend(handles=[lns1, lns2, lns3], prop={'size': 20})
plt.savefig("Fig 7(a).png", dpi=300, bbox_inches="tight")
plt.show()
