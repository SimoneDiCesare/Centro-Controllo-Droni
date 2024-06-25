import numpy as np
import visualizzatore as vis
import singleDrone as sd
import drone as dr
import block as blk
import matTools as mt
from tqdm import tqdm

#funzioni NECESSARIE per solo per la funzione della torre di controllo
def updateTime(matrice):
    h = len(matrice)
    w = len(matrice[0])
    for i in range(h):
        for j in range(w):
            matrice[i][j] += 1
    return matrice

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
    

def assegna(block, assignment, mtx):
    free = [b for b in block if b not in assignment.values()]
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



#-----------
#DATI DI PARTENZA
#ogni casella indica un'area quadrata di 14,142135624 m x 14,142135624 m 
#velocita' del drone = 15 km/h
#per passare da una casella all'altra un drone impiega 3.4 secondi. 
#ogni STEP indica un'iterazione del codice
#in questa iterazione il drone si muove di STEP
#ogni STEP indica 3.4 secondi. 
DIM_CASELLA = 14.142135624 
LATO = 2000
dim = int((LATO+DIM_CASELLA-0.000000001)/DIM_CASELLA) #per problema originale dim = 300
common_time = 1061 #per problema originale dim = 1061
Ndrones = 128
origin = (int((dim-1)/2),int((dim-1)/2)) #presumendo che l'area sia QUADRATA
t_iter = 20000
#-----------
#versione SENZA movimenti in diagonale: ogni spostamento è marcato sulla mappa
def control(mat):
    drones = [dr.Drone(common_time, origin, d) for d in range(Ndrones)]
    blocksIndex = biBlock(dim, Ndrones)
    block = [blk.Block((b[0],b[1]),(b[2], b[3]),origin) for b in blocksIndex]
    recharge = [0 for d in drones]
    dati = np.zeros((3,t_iter+1), dtype= float)
    assignment = dict()
    

    if block == []:
        return "troppi droni per la dimensione dell'area"
    iteration = 0 
    while iteration < t_iter:
        
        for r in [dro for dro in drones if dro.state == "Ready"]:
            assignment[r] = assegna(block, assignment,mat)
            r.state = "Starting"
            
        for d in drones:
            if d.state == "Dead": continue
            if d.state == "Charging": 
                recharge[d.id] += 1
                if recharge[d.id] == d.chargetime:
                    recharge[d.id] = 0
                    d.state = "Ready"
                    d.time_of_fly = common_time
                
            else:
                mat[d.position] = 0
                if d.distance(origin) >= d.time_of_fly: #se il drone è scarico rientra
                    d.position = sd.go_to(d.position, origin)
                elif d.state == "Starting": 
                    d.position = sd.go_to(d.position,assignment[d].lastvisit)
                elif d.state == "Flying":
                    new_pos = sd.scan(d.position, assignment[d], mat)

                    if new_pos == (-1,-1):
                        assignment[d].lastvisit = assignment[d].start
                        assignment[d] = None
                        assignment[d] = assegna(block, assignment,mat)
                        d.state = "Starting"
                        d.position = sd.go_to(d.position,assignment[d].start)
                    else:
                        d.position = new_pos
                        assignment[d].lastvisit = d.position
                
                #Update state
                if d.position == assignment[d].start or d.position == assignment[d].lastvisit: #se il drone è allo start "Starting"->"Flying"
                    d.state = "Flying"
                if(d.time_of_fly == 0):
                    if (d.position == origin):
                        d.state = "Charging"
                        assignment[d] = None
                    else:
                        d.state = "Dead"
                        print(f"Il drone {d.id} è morto nel punto:{d.position}")
                d.time_of_fly-=1
            #print(d, f"blocco: {assignment[d]}" )
        #print(recharge)
        #vis.visualizza(mat)
        mat = updateTime(mat)
        avg,maxx = mt.info(mat)
        in_volo = 0
        for d in drones:
            if d.state == "Starting" or d.state == "Flying":
                in_volo += 1
        dati[0,iteration] = avg
        dati[1,iteration] = maxx
        dati[2,iteration] = in_volo
        #print(f"it: {iteration}, avg: {avg}, max: {maxx}, in volo: {in_volo}")
        iteration+= 1
    print(f"avg: {sum(dati[0])/t_iter}, max:{max(dati[1])}, in volo:{sum(dati[2])/t_iter}")
    vis.salva(mat)
#versione CON movimenti in diagonale: i movimenti in diagonale non modificano la mappa ne' la matrice.
def controlD(mat):
    drones = [dr.Drone(common_time, origin, d) for d in range(Ndrones)]
    blocksIndex = biBlock(dim, Ndrones)
    block = [blk.Block((b[0],b[1]),(b[2], b[3]),origin) for b in blocksIndex]
    recharge = [1061*4 for d in drones]
    dati = np.zeros((3,t_iter+1), dtype= float)
    assignment = dict()
    diagonal = [0 for d in drones]
    destination = [(0,0) for d in drones]
    

    if block == []:
        return "troppi droni per la dimensione dell'area"
    for b in block:
        print(b)
    print(len(block))
    
    for iteration in tqdm(range(t_iter), desc="peni"):
        
        for r in [dro for dro in drones if dro.state == "Ready"]:
            assignment[r] = assegnaMax( block, assignment,mat)
            r.state = "Diagonal"
            destination[r.id] = assignment[r].lastvisit
            diagonal[r.id] = calculate_time_diag(r.position, destination[r.id])

            
        for d in drones:
            if d.state == "Dead": continue
            if d.state == "Charging": 
                if recharge[d.id] == d.chargetime:
                    recharge[d.id] = 0
                    d.state = "Ready"
                    d.time_of_fly = common_time
                recharge[d.id] += 1
                
                
            else:
                
                if calculate_time_diag(d.position, origin) >= d.time_of_fly and d.state != "Diagonal": #se il drone è scarico rientra
                    assignment[d] = None
                    d.state = "Diagonal"
                    destination[d.id] = origin
                    diagonal[d.id] = d.time_of_fly
                    #print(f"d.id: {d.id}, tof: {d.time_of_fly}, diag:{diagonal[d.id]}")

                    
                elif d.state == "Diagonal": 
                    diagonal[d.id]-=1   
                    if diagonal[d.id] == 0:
                        d.position = destination[d.id]
                        d.state = "Flying"
                    
                elif d.state =="Transferring":
                    mat[d.position] = 0
                    d.position = sd.go_to(d.position,assignment[d].lastvisit)
                    

                elif d.state == "Flying":
                    mat[d.position] = 0
                    new_pos = sd.scan(d.position, assignment[d], mat)

                    if new_pos == (-1,-1):
                        assignment[d].lastvisit = assignment[d].start
                        assignment[d] = None
                        assignment[d] = assegnaMax(block, assignment,mat)
                        d.state = "Transferring"
                        d.position = sd.go_to(d.position,assignment[d].lastvisit)
                    else:
                        d.position = new_pos
                        assignment[d].lastvisit = d.position
                
                #Update state
                if assignment[d] != None:
                    if d.position == assignment[d].start or d.position == assignment[d].lastvisit: #se il drone è allo start "Starting"->"Flying"
                        d.state = "Flying"
                if(d.time_of_fly == 0):
                    if (d.position == origin):
                        d.state = "Charging"
                    else:
                        d.state = "Dead"
                        print(f"Il drone {d.id} è morto nel punto:{d.position}")
                d.time_of_fly-=1
            #print(d, f"blocco: {assignment[d]}, diag: {diagonal[d.id]}" )
        #print(recharge)
        #vis.visualizza(mat)
        mat = mat + 1
        avg,maxx = mt.info(mat)
        in_volo = 0
        for d in drones:
            if d.state == "Starting" or d.state == "Flying":
                in_volo += 1
        dati[0,iteration] = avg
        dati[1,iteration] = maxx
        dati[2,iteration] = in_volo
        #print(f"it: {iteration}, avg: {avg}, max: {maxx}, in volo: {in_volo}")
        
    print(f"avg: {(sum(dati[0])/t_iter)*1.7}, max:{(max(dati[1]))*1.7/3600}, in volo:{sum(dati[2])/t_iter}")
    vis.salva(mat)


if __name__ == "__main__":
    mat = np.full((dim, dim), 1)
    controlD(mat)
            
    """blks = create_bloks(dim, 16)
    print(blks)
    for y in range(dim):
        for x in range(dim):
            mat[y,x] = whitchBlock(blks, (y,x))"""
    """b = [dr.Drone(10, (0,0)), dr.Drone(10, (0,1)), dr.Drone(10, (1,0))]
    a = [blk.Block((0,0),(1,1)), blk.Block((1,1),(2,2)), blk.Block((1,0),(2,1))]
    dic = dict(zip(b, a))
    print(dic[b[0]].start)"""
    
