#ifndef AREA_HPP
#define AREA_HPP
#include <vector>
#include <string>

/**
 * @class Block
 * @brief This class represent a Block by which an Area can be divided.
 * @see {Area}
 */
class Block {
    public:
        /**
         * @brief Costructor of class Block.
         * @param x The x location.
         * @param y The y location.
         * @param width The width of the block.
         * @param height The height of the block.
         */
        Block(int x, int y, int width, int height);
        /**
         * @brief Destructor of class Block.
         */
        ~Block();
        /**
         * @return The x location of this block.
         */
        int getX();
        /**
         * @return The y location of this block.
         */
        int getY();
        /**
         * @return The width of this block.
         */
        int getWidth();
        /**
         * @return The height of this block.
         */
        int getHeight();
        /**
         * @return The last visited x location of this block.
         */
        int getLastX();
        /**
         * @return The last visited y location of this block.
         */
        int getLastY();
        /**
         * @return The direction in which the x of this block are visited.
         */
        int getDirX();
        /**
         * @return The direction in which the y of this block are visited.
         */
        int getDirY();
        /**
         * @return The start x location of this block. 
         */
        int getStartX();
        /**
         * @return The start y location of this block. 
         */
        int getStartY();
        /**
         * @return True if this block is assigned to a drone.
         */
        bool isAssigned();
        /**
         * @brief Orient this block towards a center.
         * @param facingX the x center location.
         * @param facingY the y center location.
         */
        void orientBlock(int facingX, int facingY);
        /**
         * @brief Change this block x.
         * @param x The new x location.
         */
        void setX(int x);
        /**
         * @brief Change this block y.
         * @param y The new y location.
         */
        void setY(int y);
        /**
         * @brief Change this block width.
         * @param width The new width.
         */
        void setWidth(int width);
        /**
         * @brief Change this block height.
         * @param height The new height.
         */
        void setHeight(int height);
        /**
         * @brief Change this block lastX.
         * @param lastX The new lastX location.
         */
        void setLastX(int lastX);
        /**
         * @brief Change this block lastY.
         * @param lastY The new lastY location.
         */
        void setLastY(int lastY);
        /**
         * @brief Change this block dirX.
         * @param dirX The new dirX.
         */
        void setDirX(int dirX);
        /**
         * @brief Assign this block to a drone with its id.
         * @param id The drone id.
         */
        void assignTo(long long id);
        /**
         * @brief Get the drone's id assigned to this block.
         *          If no drone is assigned, it return -1.
         * @return The drone's id assigned.
         */
        long long getAssignment();
        /**
         * @brief Reset this block for revisiting in future.
         * @param facingX the center x location.
         * @param facingY the center y location.
         */
        void reset(int facingX, int facingY);
        /**
         * @brief Represent this block as a text.
         * @return This block as text.
         */
        std::string toString();
    private:
        int x;              ///< Matrix x position
        int y;              ///< Matrix y position
        int width;          ///< Width of the Block
        int height;         ///< Height of the Block
        int startX;         ///< Start x of the block
        int startY;         ///< Startx y of the block
        int dirX;           ///< X movement inside this block
        int dirY;           ///< Y movement inside this block
        int lastX;          ///< Last x visited relative to the block
        int lastY;          ///< Last y visited relative to the block
        long long droneId;  ///< Checks if this block is used or free
};


/**
 * @class Area
 * @brief This class represent a grid of int that can be divided into blocks.
 */
class Area {
    public:
        /**
         * @brief Costructor of class Area.
         * @param width The area width.
         * @param height The area height.
         */
        Area(int width, int height);
        /**
         * @brief Destructor of class Area.
         */
        ~Area();
        /**
         * @brief Divides this area into blocks oriented towards a center.
         * @param blockNumbers The number of blocks in which the area shoul be divided.
         * @param centerX The x center location.
         * @param centerY The y center location.
         * 
         * This method approximate the blocks in which this area should be divided to the neares int to blockNumbers.
         * The center is used for orientating the blocks towards it.
         */
        void initArea(int blockNumbers, int centerX, int centerY);
        /**
         * @brief Represents this area as a text.
         * @return A rapresentation of this area as text.
         */
        std::string toString();
        long long* operator[](int index) {
            return this->matrix[index];
        }
        /**
         * @return The blocks in which this area is divided.
         */
        std::vector<Block>* getBlocks();
        /**
         * @return The min value inside a block.
         */
        int getMinIn(Block block);
        /**
         * @return The max value inside a block.
         */
        int getMaxIn(Block block);
        /**
         * @return The matrix pf this area.
        */
        long long** getMatrix();
    private:
        int width;                  ///< The width of the area.
        int height;                 ///< The height of the area.
        long long** matrix;               ///< The matrix in which the area is stored.
        std::vector<Block> *blocks; ///< The blocks in which the area is divided.
};

#endif  // AREA_HPP