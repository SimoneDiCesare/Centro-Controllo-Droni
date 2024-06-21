#include "drawer.hpp"
#include <iostream>

#ifdef GUI
#include "raylib.h"

void Drawer::init(int width, int height) {
    InitWindow(1080, 720, "Grid");
}

void Drawer::drawGrid(int** grid, int width, int height) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    int cellWidth = 1080 / width;
    int cellHeight = 720 / height;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            std::cout << grid[x][y] << "\n";
            int redValue = grid[x][y];
            if (redValue >= 255) {
                redValue = 254;
            }
            int blueValue = 254 - redValue;
            Color color = { (unsigned char)(redValue), 0, (unsigned char)(blueValue), 255};
            DrawRectangle(x * cellWidth, y * cellHeight, cellWidth, cellHeight, color);
        }
    }
    EndDrawing();
}

void Drawer::close() {
    CloseWindow();
}

#else // !GUI

void Drawer::init(int width, int height) {
    std::cout << "Can't display GUI in a non gui environment\n";
}

void Drawer::drawGrid(int** grid, int width, int height) {
    // Do nothing
}


void Drawer::close() {
    // Do nothing
}

#endif // GUI