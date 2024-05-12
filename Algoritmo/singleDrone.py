'''
dir = (x,y):
    u
  l n r
    d
u = (-1,0)
l = (0,-1)
r = (0,1)
d = (1,0)
n = (0,0)
'''

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import visualizzatore 
import matTools as mt
import block as blk


#vecchia versione
"""
def check(tupla,lim):
    ex = []
    for pos in tupla:
        if( pos[0] < lim and pos[0] >= 0 and pos[1] < lim and pos[1] >= 0 ):
            ex.append(pos)
    return ex

def validNear(pos, lim):
    ips = pos[0]
    ics = pos[1]
    near = ((ips,ics-1), (ips-1,ics), (ips, ics+1),  (ips+1, ics))
    valid = check(near, lim)
    return valid

def heavierNearNextPos(mtx, ips,ics):
    valid = validNear((ips, ics),len(mtx))
    nextPos = valid[0]
    for pos in valid:
        if mtx[pos[0]][pos[1]] > mtx[nextPos[0]][nextPos[1]]:
            nextPos = pos
    return nextPos

def heavierTwoNearNextPos(mtx, ips,ics):
    dim = len(mtx)
    direction = [0,0,0,0]

    if ips-1 >= 0:
        direction[0] += mtx[ips-1][ics]
        if ips-2 >= 0:
            direction[0] += mtx[ips-2][ics]

    if ics-1 >= 0:
        direction[1] += mtx[ips][ics-1]
        if ics-2 >= 0:
            direction[1] += mtx[ips][ics-2] 

    if ips+1 < dim:
        direction[2] += mtx[ips+1][ics]
        if ips+2 < dim:
            direction[2] += mtx[ips+2][ics]
    
    if ics+1 < dim:
        direction[3] += mtx[ips][ics+1]
        if ics+2 < dim:
            direction[3] += mtx[ips][ics+2]
       
    nextdir = 0
    for i in range(4):
        if direction[i] > direction[nextdir]:
            nextdir = i
    
    print(f"{direction}, scelta: {nextdir}" )
    
    if nextdir == 0:
        return (ips -1,ics)
    elif nextdir == 1:
        return (ips, ics-1)
    elif nextdir == 2:
        return (ips+1, ics)
    
    return (ips,ics+1)

def tester(mat):
    dim = len(mat)
    origin = (int((dim-1)/2),int((dim-1)/2))
    lim = min(origin[1], origin[0], dim - origin[1] - 1, dim - origin[0] - 1)
    mt.sumup(origin, lim, mat)
    mt.sumleft(origin, lim, mat)
    mt.sumright(origin, lim, mat)
    mt.sumdown(origin, lim, mat)

def avgSliceNextPos(mtx, ips,ics):
    valid = validNear((ips, ics),len(mtx))
    if len(valid) != 4:
        return heavierNearNextPos(mtx, ips, ics)  
    dim = len(mtx)
    lim = min(ics, ips, dim - ics - 1, dim - ips - 1)

    direction =[0,0,0,0]

    direction[0] = mt.avgup((ips,ics), mtx)
    direction[1] = mt.avgleft((ips,ics), mtx)
    direction[2] = mt.avgdown((ips,ics), mtx)
    direction[3] = mt.avgright((ips,ics), mtx)

    nextdir = 3
    for i in range(4):
        if direction[i] > direction[nextdir]:
            nextdir = i
    
    print(f"{direction}, scelta: {nextdir}" )
    
    if nextdir == 0:
        return (ips -1,ics)
    elif nextdir == 1:
        return (ips, ics-1)
    elif nextdir == 2:
        return (ips+1, ics)
    return (ips,ics+1)

def heavierSliceNextPos(mtx, current, pre):
    ips, ics = current
    dim = len(mtx)
    valid = validNear((ips, ics),dim)
    if pre in valid: valid.remove(pre)
    dim = len(mtx)
    lim = min(ics, ips, dim-ics-1, dim-ips-1 )

    direction ={pos:0 for pos in valid}

    for dire in valid:
        if dire in direction:
            if dire == (ips-1,ics):
                direction[(ips-1,ics)] = mt.sumup(current, lim, mtx)
            if dire == (ips,ics-1):
                direction[(ips,ics-1)] = mt.sumleft(current, lim, mtx)
            if dire == (ips+1,ics):
                direction[(ips+1,ics)] = mt.sumdown(current, lim, mtx)
            if dire == (ips,ics+1):
                direction[(ips,ics+1)] = mt.sumright(current, lim, mtx)
    
    nextdir = list(direction.keys())[0]
    for dire in direction.keys():
        if direction[dire] > direction[nextdir]:
            nextdir = dire
    
    print(f"{direction}, scelta: {nextdir}" )
    
    return nextdir

# Algo brutto
def corner_case(pos, block, mtx):
    if block.getSize() == 1: #specal case when block is 1x1
        return pos
    vNear = [] 
    if pos[1] > block.up_left[1]:
        vNear.append((pos[0], pos[1]-1))
    if pos[1] < block.down_right[1]-1:
        vNear.append((pos[0], pos[1]+1))
    if pos[0] > block.up_left[0]:
        vNear.append((pos[0]-1, pos[1]))
    if pos[0] < block.down_right[0]-1:
        vNear.append((pos[0]+1, pos[1]))

    nextPos = vNear[0]

    for p in vNear:
        if mtx[p[0]][p[1]] > mtx[nextPos[0]][nextPos[1]]:
            nextPos = p
    return nextPos

def scam(pos, block, mtx):
    if pos[1] == block.up_left[1] or pos[1] == block.down_right[1]-1:
        return corner_case(pos, block, mtx)
    if mtx[pos[0]][pos[1]+1] > mtx[pos[0]][pos[1]-1]:
        return (pos[0], pos[1]+1)
    else:
        return (pos[0], pos[1]-1)
"""
#Last version  
def scan(pos, block, mtx):
    if block.getSize() == 1: #specal case when block is 1x1
        return pos

    if pos[1] == block.up_left[1]:
        if block.start == block.up_left:
            if block.getWidth() == 1:
                if pos[0]!= block.down_right[0]:
                    return(pos[0]+1, pos[1])
                else: 
                    return(pos[0]-1, pos[1])
            
            
            if mtx[pos[0], pos[1]+1] ==1: #vieni da destra
                if pos[0] != block.down_right[0]-1: 
                    return (pos[0]+1, pos[1])
                else: 
                    return (-1,-1)
            else:
                return (pos[0], pos[1]+1)
        else: #start = down right 
            if block.getWidth() == 1:
                if pos[0]!= block.up_left[0]:
                    return(pos[0]-1, pos[1])
                else: 
                    return(pos[0]+1, pos[1])
            if mtx[pos[0], pos[1]+1] ==1: #vieni da destra
                if pos[0] != block.up_left[0]: 
                    return (pos[0]-1, pos[1])
                else: #sei al limite superiore

                    return (-1,-1) #hai terminato, non puoi più salire
            else:
                return (pos[0], pos[1]+1)
    
    if pos[1] == block.down_right[1]-1:
        if block.start[0] == block.down_right[0]-1:

                if mtx[pos[0], pos[1]-1] == 1: #vieni da sinistra
                    if pos[0] != block.up_left[0]: 
                        return (pos[0]-1, pos[1])
                    else: 
                        return (-1,-1)
                else:
                    return (pos[0], pos[1]-1)
        else: #start = up_left

            if mtx[pos[0], pos[1]-1] ==1: #vieni da sinistra
                if pos[0] != block.down_right[0]-1: 
                    return (pos[0]+1, pos[1])
                else: #sei al limite superiore
                    return (-1,-1) #hai terminato, non puoi più salire
            else:
                return (pos[0], pos[1]-1)

    if mtx[pos[0]][pos[1]+1] > mtx[pos[0]][pos[1]-1]:
        return (pos[0], pos[1]+1)
    else:
        return (pos[0], pos[1]-1)



def go_to(pos, origin):
    if pos[1] > origin[1]:
        return pos[0], pos[1]-1
    if pos[1] < origin[1]:
        return pos[0], pos[1]+1
    if pos[0] > origin[0]:
        return pos[0]-1, pos[1]
    if pos[0] < origin[0]:
        return pos[0]+1, pos[1]
    
    return pos



if __name__ == "__main__":

    dim = 20
    mat = [[5 for i in range(dim)] for i in range(dim)]
    
    #tester(mat)
    #print(mat)
    
    

        


    

