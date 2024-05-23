#ifndef AREA_HPP
#define AREA_HPP
#include <vector>
#include <string>

class Block {
    public:
        Block(int x, int y, int width, int height);
        ~Block();
        int getX();
        int getY();
        int getWidth();
        int getHeight();
        int getLastX();
        int getLastY();
        int getDirX();
        int getDirY();
        int getStartX();
        int getStartY();
        void orientBlock(int facingX, int facingY);
        void setX(int x);
        void setY(int y);
        void setWidth(int width);
        void setHeight(int height);
        std::string toString();
    private:
        int x;         ///< Matrix x position
        int y;         ///< Matrix y position
        int width;     ///< Width of the Block
        int height;    ///< Height of the Block
        int startX;    ///< Start x of the block
        int startY;    ///< Startx y of the block
        int dirX;      ///< X movement inside this block
        int dirY;      ///< Y movement inside this block
        int lastX;     ///< Last x visited relative to the block
        int lastY;     ///< Last y visited relative to the block
};

class Area {
    public:
        Area(int width, int height);
        ~Area();
        void initArea(int blockNumbers, int centerX, int centerY);
        std::string toString();
        int*& operator[](int index);
    private:
        int width;
        int height;
        int** matrix;
        std::vector<Block> *blocks;
};

#endif