from sys import stdin

# steps = 1000000 * 20
steps = 124836180 * 20
load = 0
walk_step = 0

step_seq = []
load_seq = []

for line in stdin:
    if "load" in line:
        if walk_step > 0:
            step_seq.append(walk_step)
        walk_step = 0
        load += 1
    else:
        if load > 0:
            load_seq.append(load)
        load = 0
        walk = line.split()[-1]
        walk_step += int(walk)

step_seq.append(walk_step)

print(len(load_seq), len(step_seq))

rest_walk = [steps]
for i in range(len(load_seq)):
    for j in range(load_seq[i]):
        steps -= step_seq[i] / load_seq[i]
        rest_walk.append(steps)

print(len(rest_walk))
print(", ".join(map(str, rest_walk)))