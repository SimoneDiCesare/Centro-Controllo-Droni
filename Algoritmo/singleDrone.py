import numpy as np 



def inBlocks(block, pos):
    if pos[0]>=block.up_left[0] and pos[1]>=block.up_left[1] and pos[0]< block.down_right[0] and pos[1]<block.down_right[1] :
        return True
    return False


def whitchBlock(blocks,pos):
    for i in range(len(blocks)):
        if inBlocks(blocks[i],pos):
            return i
    return -1


def max_block(blocks):
    g = 0
    for i in range(len(blocks)):
        if (blocks[i][2]-blocks[i][0])*(blocks[i][3]-blocks[i][1]) > (blocks[g][2]-blocks[g][0])*(blocks[g][3]-blocks[g][1]) :
            g = i
    return g


def size(block): return ((block.down_right[0]-block.up_left[0])*(block.down_right[1]-block.up_left[1]))


def create_blocks(start_dim, nblocks):
    #list of blocks:
    #[...
    # i-esimo blocco: [inizio_x, inizio_y, fine_i, fine_y],
    #    ...]
    if nblocks > start_dim*start_dim:
        return []
    blocks = [[0,0,start_dim,start_dim]]
    while len(blocks) < nblocks:
        #print("max: ", max_block(blocks)," in: ", blocks)
        greater = blocks.pop(max_block(blocks))
        
        if greater[2]-greater[0] <= greater[3] - greater[1]: #più largo che lungo 
            #print("più largo che lungo")
            cupple = [[greater[0], greater[1], greater[2], greater[1]+int((greater[3]-greater[1]+1)/2)], [greater[0], greater[1]+int((greater[3]-greater[1]+1)/2), greater[2], greater[3]]]
        else: #più lungo che largo
            #print("più lungo che largo ")
            
            cupple = [[greater[0], greater[1], greater[0]+int((greater[2]-greater[0]+1)/2), greater[3]], [greater[0]+int((greater[2]-greater[0]+1)/2), greater[1], greater[2], greater[3]]]
        blocks.append(cupple[0])
        blocks.append(cupple[1])
    return blocks


def biBlock(start_dim, nblock):
    if nblock == 0: return -1
    pow_of_two = 1
    while pow_of_two < nblock:
       pow_of_two *= 2
       if pow_of_two >= start_dim*start_dim:
           return create_blocks(start_dim, start_dim**2)
    return create_blocks(start_dim, pow_of_two)


def approxBlock(nDrones):
   
    h= int(np.ceil(np.sqrt(nDrones)))
    if h == 0 : h = 1
    w = h 
    
    while w * h < nDrones:
        if (w+1)*h <= nDrones: w += 1
        else:h += 1
    print("blocchi in altezza:",h,"blocchi in larghezza:",w)
    return h, w


def create_blocks_2(area_dim, nDrones):
    blocks = []
    if nDrones >= area_dim**2:
        h = area_dim
        w = area_dim
    else: h,w = approxBlock(nDrones)
    heightBlock = int((area_dim + (h -1))/h)
    widthBlock = int((area_dim + (w -1))/w)
    wasteHD = h*heightBlock - area_dim
    wasteWR = w*widthBlock - area_dim
    wasteHU = 0
    wasteWL = 0
    if wasteHD > heightBlock : 
        wasteHU = wasteHD//2
        wasteHD -= wasteHU
    if wasteWR > heightBlock : 
        wasteWL = wasteWR//2
        wasteWR -= wasteWL
    print("heightBlock:",heightBlock,", widthBlock:", widthBlock)
    print("wasteHU:", wasteHU,", wasteWL",wasteWL)
    print("wasteHD:",wasteHD,", wasteWR:",wasteWR)
    blocks.append([0,0,heightBlock-wasteHU, widthBlock-wasteWL])
    for y in range(h-1):
        for x in range(w-1):
            if x == 0 and y == 0: continue
            inizio_y = heightBlock*y - wasteHU
            if inizio_y < 0: inizio_y = 0
            inizio_x = widthBlock*x - wasteWL
            if inizio_x < 0: inizio_x = 0
            blocks.append([inizio_y, inizio_x, heightBlock+inizio_y, widthBlock+inizio_x])

    offsetInizio_x = widthBlock*(w-1)-wasteWL
    offsetFine_x = offsetInizio_x+widthBlock-wasteWR
    for y in range(h-1):
        inizio_y = heightBlock*y - wasteHU
        if inizio_y < 0: inizio_y = 0
        if inizio_y == 0 and offsetInizio_x == 0 : continue
        blocks.append([inizio_y, offsetInizio_x, inizio_y + heightBlock, offsetFine_x])

    offsetInizio_y = heightBlock*(h-1)-wasteHU
    offsetFine_y = offsetInizio_y+heightBlock-wasteHD
    for x in range(w-1):
        inizio_x = widthBlock*x - wasteWL
        if inizio_x < 0: inizio_x = 0
        if offsetInizio_y == 0 and inizio_x == 0 : continue
        blocks.append([offsetInizio_y, inizio_x, offsetFine_y, inizio_x+widthBlock])

    blocks.append([offsetInizio_y,offsetInizio_x,offsetFine_y,offsetFine_x])
    return blocks    


def assegna(block, assignment, mtx):
    free = [b for b in block if b not in assignment.values()]
    if free == []: return None
    res = free[0]
    res_val = res.val(mtx)
    for bl in free:
        bl_val = bl.val(mtx)
        if bl_val > res_val:
            res_val = bl_val
            res = bl
    return res
    

def assegnaMax(block, assignment, mtx):
    free = [b for b in block if b not in assignment.values()]
    if free == []: return None
    res = free[0]
    res_max = mtx[res.up_left[0]:res.down_right[0], res.up_left[1]:res.down_right[1]].max()
    for bl in free:
        bl_max = mtx[bl.up_left[0]:bl.down_right[0], bl.up_left[1]:bl.down_right[1]].max()
        if bl_max > res_max:
            res_max = bl_max
            res = bl
    return res


def calculate_time_diag(a,b):
    return int(np.sqrt(((a[0]-b[0])**2)+(a[1]-b[1])**2)+1)


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
    
    

        


    

