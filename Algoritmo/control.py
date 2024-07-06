import numpy as np
import visualizzatore as vis
from singleDrone import assegnaMax, calculate_time_diag, create_blocks_2, go_to, scan, assegna, biBlock
import drone as dr
import block as blk
import matTools as mt
from tqdm import tqdm

#DATI DI PARTENZA
#ogni casella indica un'area quadrata di 14,142135624 m x 14,142135624 m 
#velocita' del drone = 15 km/h
#per passare da una casella all'altra un drone impiega 3.4 secondi. 
#ogni STEP indica un'iterazione del codice
#in questa iterazione il drone si muove di STEP
#ogni STEP indica 3.4 secondi. 

visualizza = False #per visualizzare l'area visitata a runtime impostare <False> per ottenere direttamente il risultato finale impostare <True> 
Ndrones = 500
LATO = 6000 # in metri
tempo_simulato = 12 # in ore 
common_time_of_fly = 30 # tempo di volo comune a tutti i droni in minuti
DIM_CASELLA = 10*np.sqrt(2)


t_iter = int(np.ceil(tempo_simulato*3600/1.7)) # numero di iterazioni    
common_time = int(np.ceil(common_time_of_fly * 60 /1.7)) 
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
                    d.position = go_to(d.position, origin)
                elif d.state == "Starting": 
                    d.position = go_to(d.position,assignment[d].lastvisit)
                elif d.state == "Flying":
                    new_pos = scan(d.position, assignment[d], mat)

                    if new_pos == (-1,-1):
                        assignment[d].lastvisit = assignment[d].start
                        assignment[d] = None
                        assignment[d] = assegna(block, assignment,mat)
                        d.state = "Starting"
                        d.position = go_to(d.position,assignment[d].start)
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
           
        mat = mat+1
        avg,maxx = mt.info(mat)
        in_volo = 0
        for d in drones:
            if d.state == "Starting" or d.state == "Flying":
                in_volo += 1
        dati[0,iteration] = avg
        dati[1,iteration] = maxx
        dati[2,iteration] = in_volo
        iteration+= 1
    print(f"avg: {sum(dati[0])/t_iter}, max:{max(dati[1])}, in volo:{sum(dati[2])/t_iter}")
    vis.salva(mat)


#versione CON movimenti in diagonale: i movimenti in diagonale non modificano la mappa ne' la matrice.
def controlD(mat):
    drones = [dr.Drone(common_time, origin, d) for d in range(Ndrones)]
    average_charge = np.mean(np.array([d.chargetime for d in drones]))
    average_time_of_fly = np.mean(np.array([d.time_of_fly for d in drones]))
    n_sets = int((average_charge + average_time_of_fly-1)/average_time_of_fly +1)
    batteria = Ndrones//n_sets
    if batteria == 0: batteria = 1
    blocksIndex = create_blocks_2(dim, Ndrones//n_sets)
    block = [blk.Block((b[0],b[1]),(b[2], b[3]),origin) for b in blocksIndex]
    recharge = [1061*4 for d in drones]
    dati = np.zeros((3,t_iter+1), dtype= float)
    assignment = dict()
    for d in drones:
        assignment[d] = None
    diagonal = [0 for d in drones]
    destination = [(0,0) for d in drones]
    if block == []:
        return "troppi droni per la dimensione dell'area"
    print(f"Area: {LATO} km^2 / {dim}x{dim} matrix\ndroni totali: {Ndrones}, \n\taverage_charge: {average_charge}, average_time_of_fly: {average_time_of_fly}, n_sets:{n_sets} \nnumero di blocchi: {len(block)}")
    for iteration in tqdm(range(t_iter),desc="iterazioni"):
       
        for r in [dro for dro in drones if dro.state == "Ready"]:
            assignment[r] = assegnaMax(block, assignment,mat)
            if assignment[r] != None:
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
                if d.state != "Ready":
                    
                    if calculate_time_diag(d.position, origin) >= d.time_of_fly and d.state != "Diagonal": #se il drone è scarico rientra
                        assignment[d] = None
                        d.state = "Diagonal"
                        destination[d.id] = origin
                        diagonal[d.id] = d.time_of_fly
                        

                        
                    elif d.state == "Diagonal": 
                        diagonal[d.id] -= 1   
                        if diagonal[d.id] == 0:
                            d.position = destination[d.id]
                            d.state = "Flying"
                        
                    elif d.state =="Transferring":
                        mat[d.position] = 0
                        d.position = go_to(d.position,assignment[d].lastvisit)
                        

                    elif d.state == "Flying":
                        mat[d.position] = 0
                        new_pos = scan(d.position, assignment[d], mat)

                        if new_pos == (-1,-1):
                            assignment[d].lastvisit = assignment[d].start
                            assignment[d] = None
                            assignment[d] = assegnaMax(block, assignment,mat)
                            d.state = "Transferring"
                            d.position = go_to(d.position,assignment[d].lastvisit)
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
         
        if visualizza : vis.visualizza(mat)
        mat = mat + 1
        avg = np.mean(mat)
        maxx = np.max(mat)
        in_volo = 0
        for d in drones:
            if d.state == "Starting" or d.state == "Flying":
                in_volo += 1
        dati[0,iteration] = avg
        dati[1,iteration] = maxx
        dati[2,iteration] = in_volo
       
    aver = np.mean(dati[0])*1.7
    last_Max = np.max(dati[1])*1.7
    print(f"avg: {int(aver//3600)}h, {int((aver%3600)//60)}m, {int(aver%60)}s , max: {int(last_Max//3600)}h, {int((last_Max%3600)//60)}m, {int(last_Max%60)}s, in volo:{np.mean(dati[2])}")
    vis.salva(mat,f"salvataggi/{Ndrones}_{LATO}_{tempo_simulato}.png")
    return [aver/60, last_Max/60]


if __name__ == "__main__":
    res = []
    aree = [1000]
    numero_di_droni = [1,64,256]
    for a in aree:
        for nud in numero_di_droni:
            print(a, nud)
            Ndrones = nud
            LATO = a
            dim = int(np.ceil(LATO/DIM_CASELLA))
            origin = (int((dim-1)/2),int((dim-1)/2)) #presumendo che l'area sia QUADRATA
            mat = np.full((dim, dim), 1)
            res.append([a,nud]+controlD(mat))
    np.savetxt('matrix.csv', np.array(res), delimiter=',', fmt='%d')
    
  
    
