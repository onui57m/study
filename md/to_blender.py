# @Author: Mizuki Onui <onui_m>
# @Date:   2020-10-15T00:29:00+09:00
# @Last modified by:   onui_m
# @Last modified time: 2020-10-15T02:07:55+09:00

import bpy

file_name = bpy.path.abspath("//") + '\\init.dat'
fdata = open(file_name,'r')

n = int((fdata.readline().split())[0])
box_x, box_y, box_z = map( float, (fdata.readline().split()) )
vertices = []

for i in range(n):
    vertices.append(list(map( float, (fdata.readline().split()) )))
    bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=8,radius=0.5, enter_editmode=False, location=(vertices[i][0],vertices[i][1],vertices[i][2]))

fdata.close()


import bpy

file_name = bpy.path.abspath("//") + '\\init.dat'
fdata = open(file_name,'r')

n = int((fdata.readline().split())[0])
box_x, box_y, box_z = map( float, (fdata.readline().split()) )
vertices = []

for i in range(n):
    vertices.append(list( map( float, (fdata.readline().split()) ) ))

fdata.close()
