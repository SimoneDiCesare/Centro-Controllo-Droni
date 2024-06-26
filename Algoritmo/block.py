import matTools as mt 
import numpy as np
class Block:
    
    def __init__(self, ul, dr, origin):
        self.up_left = ul
        self.down_right = dr
        # Punto più lontano dalla torre
        self.start = mt.farther(ul, dr, origin)
        self.lastvisit = self.start

    def getSize(self):
        return (self.down_right[0]-self.up_left[0])*(self.down_right[1]-self.up_left[1])
    def getHight(self):
        return self.down_right[0]-self.up_left[0]
    def getWidth(self):
        return self.down_right[1]-self.up_left[1]
    def val(self, mat):
        return np.sum(mat[self.up_left[0]:self.down_right[0],self.up_left[1]:self.down_right[1]])
    def __str__(self) -> str:
        return f"[{self.up_left, self.down_right}]"

    