#ifndef DRAWER_HPP
#define DRAWER_HPP

class Drawer {
    public:
        static void init();
        static bool shouldClose();
        static void drawGrid(int** grid, int width, int height);
        static void close();

};

#endif // DRAWER_HPP