dic={}
#f=open(r'/dataset/sl/test.txt')
#f=open(r'/dataset/sl/sowalker_512/soc-LiveJournal1_path.txt')
f=open(r'/dataset/fr/sowalker_512/com-friendster.ungraph_path.txt')
for line in f:
    currentline=line.strip()
    preindex=0
    curindex=currentline.find("->")
    nextindex=currentline.find("->",curindex+1,)
    a=0
    b=0
    for i in range(currentline.count("->")-1):
        a = currentline[preindex+2:curindex]
        b = currentline[curindex+2:nextindex]
        if dic.get((a, b)):
           dic[(a,b)]+=1
        else:
           dic[(a, b)] = 1
        preindex = curindex
        curindex = nextindex
        nextindex = currentline.find("->", curindex + 1, )
import csv
header = ['source', 'destination', 'count']
with open('fr_score.csv', 'w', encoding='UTF8', newline='') as f:
    writer = csv.writer(f)
    # write the header
    writer.writerow(header)
    # write the data
    for key in dic.keys():
        writer.writerow([int(key[0]),int(key[1]),dic[key]])
