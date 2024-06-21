#include "area.hpp"
#include "log.hpp"
#include <vector>
#include <string>
#include <cmath>
#include <iostream>

Block::Block(int x, int y, int width, int height) {
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->dirX = 1;
    this->dirY = 1;
    this->droneId = -1;
}

Block::~Block() {

}

void Block::orientBlock(int facingX, int facingY) {
    int pointX[4] = {this->x, this->x, this->x + width - 1, this->x + this->width - 1};
    int pointY[4] = {this->y, this->y + this->height - 1, this->y, this->y + this->height - 1};
    int maxDist = 0;
    for (int i = 0; i < 4; i++) {
        int dist = ((facingX - pointX[i]) * (facingX - pointX[i]));
        dist = dist + ((facingY - pointY[i]) * (facingY - pointY[i]));
        if (dist > maxDist) {
            maxDist = dist;
            this->startX = pointX[i];
            this->startY = pointY[i];
        }
    }
    this->dirX = (this->startX == this->x)? 1 : -1;
    this->dirY = (this->startY == this->y)? 1 : -1;
    this->lastX = this->startX;
    this->lastY = this->startY;
}

void Block::reset(int facingX, int facingY) {
    this->orientBlock(facingX, facingY);
    this->droneId = -1;
}

int Block::getX() {
    return this->x;
}

int Block::getY() {
    return this->y;
}

int Block::getWidth() {
    return this->width;
}

int Block::getHeight() {
    return this->height;
}

int Block::getLastX() {
    return this->lastX;
}

int Block::getLastY() {
    return this->lastY;
}

int Block::getDirX() {
    return this->dirX;
}

int Block::getDirY() {
    return this->dirY;
}

int Block::getStartX() {
    return this->startX;
}

int Block::getStartY() {
    return this->startY;
}

bool Block::isAssigned() {
    return this->droneId != -1;
}

long long Block::getAssignment() {
    return this->droneId;
}

void Block::setX(int x) {
    this-> x = x;
}

void Block::setY(int y) {
    this->y = y;
}

void Block::setWidth(int width) {
    this->width = width;
}

void Block::setHeight(int height) {
    this->height = height;
}

void Block::setLastX(int lastX) {
    this->lastX = lastX;
}

void Block::setLastY(int lastY) {
    this->lastY = lastY;
}

void Block::setDirX(int dirX) {
    this->dirX = dirX;
}

void Block::assignTo(long long droneId) {
    this->droneId = droneId;
}

std::string Block::toString() {
    return "{x:" + std::to_string(this->x) + ",y:" + std::to_string(this->y) + ",width:" + std::to_string(this->width) + ",height:" + std::to_string(this->height) + "}";

}

Area::Area(int width, int height) {
    this->width = width;
    this->height = height;
    this->matrix = new int*[this->width];
    for (int i = 0; i < this->width; i++) {
        this->matrix[i] = new int[this->height];
        for (int j = 0; j < this->height; j++) {
            this->matrix[i][j] = 0;
        }
    }
}

void calculateBlocksInArea(int width, int height, int &numBlocks, int &blockWidth, int &blockHeight) {
    int w = std::sqrt(numBlocks);
    int h = w;
    while (h * w < numBlocks) {
        if ((h + 1) * w <= numBlocks) {
            h++;
        } else {
            w++;
        }
    }
    numBlocks = w * h;
    blockWidth = std::ceil(static_cast<double>(width) / w);
    blockHeight = std::ceil(static_cast<double>(height) / h);
}

void Area::initArea(int blockCount, int centerX, int centerY) {
    this->blocks = new std::vector<Block>();
    int blockWidth = 0;
    int blockHeight = 0;
    calculateBlocksInArea(this->width, this->height, blockCount, blockWidth, blockHeight);
    logDebug("Area", "Area Approximated to " + std::to_string(blockCount) + "(" + std::to_string(blockWidth) + "," + std::to_string(blockHeight) + ")");
    int x = 0;
    int y = 0;
    for (int i = 0; i < blockCount; i++) {
        Block b(x, y, blockWidth, blockHeight);
        x += blockWidth;
        if (b.getX() >= this->width) {
            b.setX(0);
            b.setY(b.getY() + blockHeight);
            x = blockWidth;
            y += blockHeight;
        }
        if (b.getY() >= this->height) {
            // Extra blocks
            break;
        }
        if (b.getX() + b.getWidth() >= this->width) {
            b.setWidth(this->width - b.getX());
        }
        if (b.getY() + b.getHeight() >= this->height) {
            b.setHeight(this->height - b.getY());
        }
        b.orientBlock(centerX, centerY);
        this->blocks->push_back(b);
    }
    logDebug("Area", "Fitted " + std::to_string(this->blocks->size()) + " blocks");
}

Area::~Area() {
    for (int i = 0; i < this->width; i++) {
        delete this->matrix[i];
    }
    delete this->matrix;
    delete this->blocks;
}

int Area::getMaxIn(Block block) {
    int max = 0;
    for (int i = block.getX(); i < block.getX() + block.getWidth(); i++) {
        for (int j = block.getY(); j < block.getY() + block.getHeight(); j++) {
            if (max < this->matrix[i][j]) {
                max = this->matrix[i][j];
            }
        }
    }
    return max;
}

std::string Area::toString() {
    std::string text = "{matrix:" + std::to_string(this->width) + "X" + std::to_string(this->height);
    for (int i = 0; i < this->blocks->size(); i++) {
        text = text + this->blocks->at(i).toString() + "\n";
    }
    return text + "}";
}

std::vector<Block>* Area::getBlocks() {
    return this->blocks;
}

int** Area::getMatrix() {
    return this->matrix;
}