import matTools as mt 

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
        su = 0
        for i in range(self.up_left[0], self.down_right[0]):
            for j in range(self.up_left[1], self.down_right[1]):
                su += mat[i,j]
        return su
    def __str__(self) -> str:
        return f"[{self.up_left, self.down_right}]"

    