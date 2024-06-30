import numpy as np
def info(mat):
    summ = 0
    massi = []
    for r in mat:
        summ = sum(r)
        massi.append(max(r))
    return (summ/(len(mat)**2), max(massi))




#funzioni per modificare l'intera matrice
def updateTime(matrice):
    h = len(matrice)
    w = len(matrice[0])
    for i in range(h):
        for j in range(w):
            matrice[i][j] += 1
    return matrice

# funzioni per sommare fette di matrici

def sumup(start, lim, mtx):
    summ = 0
    i = lim
    while i > 0:
        for j in range(start[1]-i, start[1] + i):
            summ += mtx[start[0]-i][j] 
            #mtx[start[0]-i][j] = 0
        i -= 1 
    return summ

def sumdown(start, lim, mtx):
    summ = 0
    i = lim
    while i > 0:
        for j in range(start[1]-i+1, start[1] + i+1):
            summ += mtx[start[0]+i][j] 
            #mtx[start[0]+i][j] = 2
        i -= 1 
    return summ

def sumright(start, lim, mtx):
    summ = 0
    j = lim
    while j > 0:
        for i in range(start[0]-j, start[0] + j):
            summ += mtx[i][start[1]+j] 
            #mtx[i][start[1]+j] = 3
        j -= 1 
    return summ

def sumleft(start, lim, mtx):
    summ = 0
    j = lim
    while j > 0:
        for i in range(start[0]-j+1, start[0] + j+1):
            summ += mtx[i][start[1]-j] 
            #mtx[i][start[1]-j] = 1

        j -= 1 
    return summ


#funzioni per avere la media del valore delle fette
def avgup(start, mtx):
    num = 0
    summ = 0
    dim = len(mtx)
    for y in range(start[0]):
        
        for x in range(max(0,start[1]-start[0] + y), min(start[1] + start[0]-y, dim)):
            num += 1
            summ += mtx[y][x]
            if (abs(y-start[0])==1):
                print("si: u")
                summ += mtx[y][x] 
            #mtx[y][x] = 0
    return summ / max(num, 1)

def avgdown(start, mtx):
    num = 0
    summ = 0
    dim = len(mtx)
    for y in range(start[0]+1, dim):
        
        for x in range(max(0, start[1] - (y - start[0]) +1), min(start[1] + (y - start[0]) +1, dim)):
            num += 1
            summ += mtx[y][x] 
            if (abs(y-start[0])==1):
                print("si: d")
                summ += mtx[y][x] 
            #mtx[y][x] = 2
    return summ / max(num, 1)

def avgright(start, mtx):
    num = 0
    summ = 0
    dim = len(mtx)
    for x in range(start[1]+1, dim):
        for y in range(max(0, start[0] - (x - start[1]) ), min(start[0] + (x - start[1]) , dim)):
            num += 1
            summ += mtx[y][x]
            if (abs(x-start[1])==1):
                print("si: r")
                summ += mtx[y][x]
            #mtx[y][x] = 3
    return summ / max(num, 1)

def avgleft(start, mtx):
    num = 0
    summ = 0
    dim = len(mtx)
    for x in range(start[0]):
        
        for y in range(max(0,start[0]-start[1] + x +1), min(start[0] + start[1]-x +1, dim)):
            num += 1
            summ += mtx[y][x] 
            if (abs(x-start[1])==1):
                print("si: l")
                summ += mtx[y][x]
            #mtx[y][x] = 1
    return summ / max(num, 1)

def tester(mat):
    dim = len(mat)
    origin = (int((dim-1)/2),int((dim-1)/2))
    lim = min(origin[1], origin[0], dim - origin[1] - 1, dim - origin[0] - 1)
    sumup(origin, lim, mat)
    sumleft(origin, lim, mat)
    sumright(origin, lim, mat)
    sumdown(origin, lim, mat)

#calocoli su punti

def farther(l,r, orig):
        dl = np.sqrt(pow(l[0]-orig[0],2)+ pow(l[1] - orig[1],2))
        dr = np.sqrt(pow(r[0]-orig[0],2)+ pow(r[1] - orig[1],2))
        if dr < dl : return l
        return (r[0]-1,r[1]-1)



if __name__ == "__main__":
    """dim = 20
    mat = [[5 for i in range(dim)] for i in range(dim)]
    start = [15,6]
    mat[start[0]][start[1]] = 1
    avgup(start, mat)
    avgdown(start, mat)
    avgright(start, mat)
    avgleft(start, mat)"""
    mati = np.arange(25).reshape((5,5))
    cu = mati[2:4, 2:4]
    print(mati, "\n", cu, "\n", cu.max(), mati.max())
