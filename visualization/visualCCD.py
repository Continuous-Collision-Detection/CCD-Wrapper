import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import mpl_toolkits.mplot3d.axes3d as p3
from matplotlib.animation import PillowWriter  
import os
import sys
import glob
import gmpy2
from gmpy2 import mpq

# below is the user interface
#################################################
# set the query file path and name
filename="../external/Sample-Queries/chain/vertex-face/data_0_0.csv"

# Edge-Edge or Vertex-Trangle
is_edge_edge=False

# the animation shows the trajectory of the motion or not
show_trajectories=True

# query id (starts from 0)
query_id=0
#################################################
addname=""
if show_trajectories:
    addname="trajectories"
# input two strings a, b, output float(a/b) 
def string_to_float(numerator,denominator):
    number_str=numerator+"/"+denominator
    rnumber=mpq(number_str,base=10)
    fnumber=float(rnumber)
    return fnumber

def get_bounding_box(vertices):
    xl=vertices[0][0]
    xr=xl
    yl=vertices[0][1]
    yr=yl
    zl=vertices[0][2]
    zr=zl
    for ver in vertices:
        if xl>ver[0]:
            xl=ver[0]
        if xr<ver[0]:
            xr=ver[0]

        if yl>ver[1]:
            yl=ver[1]
        if yr<ver[1]:
            yr=ver[1]

        if zl>ver[2]:
            zl=ver[2]
        if zr<ver[2]:
            zr=ver[2]
    x=[xl,xr]
    y=[yl,yr]
    z=[zl,zr]
    result=[x,y,z]
    return result
        


r=open(filename)
line_nbr=0
vertices=[]
truth=True
for line in r.readlines():
    # print(line)
    if line_nbr>=query_id*8 and line_nbr<=query_id*8+7:
        # deal with this line, get 3 floating-point number and a boolean value 
        strings=line.split("\n")[0]
        strings=strings.split(",")
        a=string_to_float(strings[0],strings[1])
        b=string_to_float(strings[2],strings[3])
        c=string_to_float(strings[4],strings[5])
        if strings[6]=='0':
            truth=False
        else:
            truth=True
        vertex=[a,b,c]
        vertices.append(vertex)
        



    line_nbr+=1 # the 



    # if line_nbr>3:
    #     exit(0)
if vertices==[]:
    print("query id out of range")
    exit(0)
    
if is_edge_edge:
    print("**** running edge-edge CCD")
else:
    print("**** running vertex-triangle CCD")
print("****the result of CCD is: ",str(truth))
fig = plt.figure()
ax = fig.add_subplot(projection="3d")


bounding_box=get_bounding_box(vertices)
ax.set_xlim3d(bounding_box[0])
ax.set_ylim3d(bounding_box[1])
ax.set_zlim3d(bounding_box[2])




# 8 vertices of the two edges
a0s=vertices[0]
a1s=vertices[1]
b0s=vertices[2]
b1s=vertices[3]

a0e=vertices[4]
a1e=vertices[5]
b0e=vertices[6]
b1e=vertices[7]

framenumber=51
para=np.linspace(0, 1, framenumber)
para1=para
para2=np.linspace(0, 1, 2)

#start stage of a
asx=a0s[0]+(a1s[0]-a0s[0])*para
asy=a0s[1]+(a1s[1]-a0s[1])*para
asz=a0s[2]+(a1s[2]-a0s[2])*para

#end stage of a
aex=a0e[0]+(a1e[0]-a0e[0])*para
aey=a0e[1]+(a1e[1]-a0e[1])*para
aez=a0e[2]+(a1e[2]-a0e[2])*para


#start stage of b
bsx=b0s[0]+(b1s[0]-b0s[0])*para
bsy=b0s[1]+(b1s[1]-b0s[1])*para
bsz=b0s[2]+(b1s[2]-b0s[2])*para

#end stage of a
bex=b0e[0]+(b1e[0]-b0e[0])*para
bey=b0e[1]+(b1e[1]-b0e[1])*para
bez=b0e[2]+(b1e[2]-b0e[2])*para

############################# for vertex-triangle begin
psx=np.linspace(a0s[0], a0s[0], 1)
psy=np.linspace(a0s[1], a0s[1], 1)
psz=np.linspace(a0s[2], a0s[2], 1)
pex=np.linspace(a0e[0], a0e[0], 1)
pey=np.linspace(a0e[1], a0e[1], 1)
pez=np.linspace(a0e[2], a0e[2], 1)

# a1-b0
dsx=a1s[0]+(b0s[0]-a1s[0])*para2
dsy=a1s[1]+(b0s[1]-a1s[1])*para2
dsz=a1s[2]+(b0s[2]-a1s[2])*para2
dex=a1e[0]+(b0e[0]-a1e[0])*para2
dey=a1e[1]+(b0e[1]-a1e[1])*para2
dez=a1e[2]+(b0e[2]-a1e[2])*para2

# b0-b1
esx=b0s[0]+(b1s[0]-b0s[0])*para2
esy=b0s[1]+(b1s[1]-b0s[1])*para2
esz=b0s[2]+(b1s[2]-b0s[2])*para2
eex=b0e[0]+(b1e[0]-b0e[0])*para2
eey=b0e[1]+(b1e[1]-b0e[1])*para2
eez=b0e[2]+(b1e[2]-b0e[2])*para2

# a1-b1
fsx=a1s[0]+(b1s[0]-a1s[0])*para2
fsy=a1s[1]+(b1s[1]-a1s[1])*para2
fsz=a1s[2]+(b1s[2]-a1s[2])*para2
fex=a1e[0]+(b1e[0]-a1e[0])*para2
fey=a1e[1]+(b1e[1]-a1e[1])*para2
fez=a1e[2]+(b1e[2]-a1e[2])*para2


############################# for vertex-triangle end

# edge edge
plot1=ax.plot([],[],[],"b",alpha=0.5)[0]
plot2=ax.plot([],[],[],"r",alpha=0.5)[0]

# vertex face
plot3=ax.plot([],[],[],"bo",alpha=0.5)[0]
plot4=ax.plot([],[],[],"r",alpha=0.5)[0]
plot5=ax.plot([],[],[],"r",alpha=0.5)[0]
plot6=ax.plot([],[],[],"r",alpha=0.5)[0]

eary=np.array([])
line0=[eary,eary,eary]
line1=[eary,eary,eary]
line2=[eary,eary,eary]
line3=[eary,eary,eary]
if is_edge_edge:
    lines=[ax.plot(asx, asy, asz,'b')[0],ax.plot(bsx, bsy, bsz,'r')[0]]
else:
    lines=[ax.plot(psx, psy, psz,'bo')[0],ax.plot(dsx, dsy, dsz,'r')[0],ax.plot(esx, esy, esz,'r')[0],ax.plot(fsx, fsy, fsz,'r')[0]]

def interpolate(psx,pex,i):
    line=psx+(pex-psx)*para1[i]
    return line

def draw_more(xlist,newlist):
    result=np.append(xlist,newlist)
    return result

def update_lines(i):
    if is_edge_edge:
        lines[0].set_xdata(interpolate(asx,aex,i))
        lines[0].set_ydata(interpolate(asy,aey,i))
        lines[0].set_3d_properties(interpolate(asz,aez,i))
        lines[1].set_xdata(interpolate(bsx,bex,i))
        lines[1].set_ydata(interpolate(bsy,bey,i))
        lines[1].set_3d_properties(interpolate(bsz,bez,i))
    else:
        lines[0].set_xdata(interpolate(psx,pex,i))
        lines[0].set_ydata(interpolate(psy,pey,i))
        lines[0].set_3d_properties(interpolate(psz,pez,i))
        
        lines[1].set_xdata(interpolate(dsx,dex,i))
        lines[1].set_ydata(interpolate(dsy,dey,i))
        lines[1].set_3d_properties(interpolate(dsz,dez,i))

        lines[2].set_xdata(interpolate(esx,eex,i))
        lines[2].set_ydata(interpolate(esy,eey,i))
        lines[2].set_3d_properties(interpolate(esz,eez,i))

        lines[3].set_xdata(interpolate(fsx,fex,i))
        lines[3].set_ydata(interpolate(fsy,fey,i))
        lines[3].set_3d_properties(interpolate(fsz,fez,i))

    return lines

def extend_line(asx,aex,asy,aey,asz,aez,i,line,framenumber):
    xlist=interpolate(asx,aex,i)
    ylist=interpolate(asy,aey,i)
    zlist=interpolate(asz,aez,i)
    a=draw_more(line[0],xlist)
    b=draw_more(line[1],ylist)
    c=draw_more(line[2],zlist)
    if i>=framenumber-1:
        a=eary
        b=eary
        c=eary
    
    return [a,b,c]
def update_lines_trajectories(i):
    if is_edge_edge:
        [line0[0],line0[1],line0[2]]=extend_line(asx,aex,asy,aey,asz,aez,i,line0,framenumber)      
        plot1.set_xdata(line0[0])
        plot1.set_ydata(line0[1])
        plot1.set_3d_properties(line0[2])

        
        [line1[0],line1[1],line1[2]]=extend_line(bsx,bex,bsy,bey,bsz,bez,i,line1,framenumber)
        plot2.set_xdata(line1[0])
        plot2.set_ydata(line1[1])
        plot2.set_3d_properties(line1[2])

    else:
        [line0[0],line0[1],line0[2]]=extend_line(psx,pex,psy,pey,psz,pez,i,line0,framenumber)
        plot3.set_xdata(line0[0])
        plot3.set_ydata(line0[1])
        plot3.set_3d_properties(line0[2])

        [line1[0],line1[1],line1[2]]=extend_line(dsx,dex,dsy,dey,dsz,dez,i,line1,framenumber)
        plot4.set_xdata(line1[0])
        plot4.set_ydata(line1[1])
        plot4.set_3d_properties(line1[2])

        [line2[0],line2[1],line2[2]]=extend_line(esx,eex,esy,eey,esz,eez,i,line2,framenumber)
        plot5.set_xdata(line2[0])
        plot5.set_ydata(line2[1])
        plot5.set_3d_properties(line2[2])

        [line3[0],line3[1],line3[2]]=extend_line(fsx,fex,fsy,fey,fsz,fez,i,line3,framenumber)
        plot6.set_xdata(line3[0])
        plot6.set_ydata(line3[1])
        plot6.set_3d_properties(line3[2])
        
    return 
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
ax.set_title('CCD Result, '+str(truth))


if show_trajectories:
    ani = animation.FuncAnimation(
        fig, update_lines_trajectories, interval=20,frames=framenumber)
else:
    ani = animation.FuncAnimation(
        fig, update_lines, interval=20,frames=framenumber)
    
writer = PillowWriter(fps=25)  
if is_edge_edge:
    ani.save("visual_edge_edge_"+addname+".gif", writer=writer)  
else:
    ani.save("visual_vertex_triangle_"+addname+".gif", writer=writer)
plt.show()



    