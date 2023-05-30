input_path =
csv_path=

total_steps=0
block_id=0
block_edges=0
block_id_bar=[]
block_edges_bar=[]
steps_bar=[]
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
import csv
data = ["block_id", "edge", "steps in this I/O"]
with open(csv_path, "w", encoding="gbk", newline="") as f:
    csv_writer = csv.writer(f)
    csv_writer.writerow(data)
    for i in range(len(steps_bar)):
        csv_writer.writerow([block_id_bar[i],block_edges_bar[i], steps_bar[i]])
    f.close()

