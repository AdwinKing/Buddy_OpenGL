#!/usr/bin/python

content = []
with open("bunny_iH.ply2", "r") as ply_file:
    content.extend(ply_file.readlines()[2:34836])
count_unreliable = 0
count_far = 0
max_x = 0
max_y = 0
max_z = 0
for line in content:
    vertex = [float(v) for v in line.split()]
    if abs(vertex[0]) > max_x:
        max_x = abs(vertex[0])
    if abs(vertex[1]) > max_y:
        max_y = abs(vertex[1])
    if abs(vertex[2]) > max_z:
        max_z = abs(vertex[2])
    # if vertex[3] < 0.1:
    #     count_unreliable += 1
    #     print(vertex)
    # if vertex[0]**2 + vertex[1]**2 + vertex[2]**2 > 400:
    #     print("{0};{1};{2}".format(vertex[0], vertex[1], vertex[2]))
    #     count_far += 1

print("{0};{1};{2}".format(max_x, max_y, max_z))
print(count_unreliable)
print(count_far)
