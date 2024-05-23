#include "area.hpp"
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
}

Block::~Block() {

}

void Block::orientBlock(int facingX, int facingY) {
    int pointX[4] = {this->x, this->x, this->x + width, this->x + this->width};
    int pointY[4] = {this->y, this->y + this->height, this->y, this->y + this->height};
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
    std::cout << "Area Approximated for: " << blockCount << " block of " << blockWidth << "X" << blockHeight << "\n";
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
    std::cout << "Ended with: " << this->blocks->size() << " blocks\n";
}

Area::~Area() {
    for (int i = 0; i < this->width; i++) {
        delete this->matrix[i];
    }
    delete this->matrix;
    delete this->blocks;
}

int*& Area::operator[](int index) {
    return this->matrix[index];
}

std::string Area::toString() {
    std::string text = "{matrix:" + std::to_string(this->width) + "X" + std::to_string(this->height);
    for (int i = 0; i < this->blocks->size(); i++) {
        text = text + this->blocks->at(i).toString() + "\n";
    }
    return text + "}";
}