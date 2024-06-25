#include "drawer.hpp"
#include <iostream>

#ifdef GUI
#include "raylib.h"

void Drawer::init() {
    InitWindow(1080, 720, "Grid");
}

void Drawer::drawGrid(int** grid, int width, int height) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    float cellWidth = 1080.f / width;
    float cellHeight = 720.f / height;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            // std::cout << grid[x][y] << "\n";
            int redValue = grid[x][y];
            // Map red value
            redValue *= (int)((redValue / 64.f) * 255);
            if (redValue > 255) {
                redValue = 255;
            }
            int blueValue = 255 - redValue;
            Color color = { (unsigned char)(redValue), 0, (unsigned char)(blueValue), 255};
            DrawRectangleV((Vector2){x * cellWidth, y * cellHeight}, (Vector2){cellWidth, cellHeight}, color);
            // std::cout << x * cellWidth << "," << y * cellHeight << "," << cellWidth << "," << cellHeight << "\n";
            // DrawRectangle(x * cellWidth, y * cellHeight, cellWidth, cellHeight, color);
        }
    }
    EndDrawing();
}

bool Drawer::shouldClose() {
    return WindowShouldClose();
}

void Drawer::close() {
    CloseWindow();
}

#else // !GUI

void Drawer::init() {
    std::cout << "Can't display GUI in a non gui environment\n";
}

void Drawer::drawGrid(int** grid, int width, int height) {
    // Do nothing
}

bool Drawer::shouldClose() {
    return true; // Close instanlty the loop -> no display
}

void Drawer::close() {
    // Do nothing
}

#endif // GUI