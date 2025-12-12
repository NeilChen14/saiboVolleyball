
#include "gameDisplay.h"
#include <iostream>

int main(int argc, char** argv) {
    // 创建显示管理器
    GameDisplay display(1400, 900);

    if (!display.init()) {
        std::cerr << "GameDisplay 初始化失败" << std::endl;
        return -1;
    }

    // 主循环
    SDL_Event event;
    while (display.isRunning()) {
        while (SDL_PollEvent(&event)) {
            display.handleEvent(event);
        }

        display.update();
        display.render();

        SDL_Delay(16); // ~60 FPS
    }

    return 0;
}
