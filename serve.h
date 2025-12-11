#ifndef SERVE_H
#define SERVE_H

#include "player.h"
#include "game.h"
#include <cmath>

// 发球选择枚举
enum ServeType {
    STABLE_SERVE = 1,   // 稳定发球
    AGGRESSIVE_SERVE = 2 // 冲发球
};

// 发球结果结构体
struct ServeResult {
    bool success;       // 发球是否成功
    int effectiveness;  // 发球效果值
    ServeType type;     // 发球类型
};

class Serve {
private:
    const Player& server;
    const GameState& game;
    ServeType serveType;
    double adjustment;

    ServeType decideServeStrategy();
    double calculateServeAdjustment();
    int calculateServePower();
    double calculateServeFaultRate();

public:
    Serve(const Player& player, const GameState& gameState);

    ServeResult simulate();

    ServeType getServeType() const { return serveType; }
    double getAdjustment() const { return adjustment; }


};


 // 函数声明
// ServeResult simulateServe(const Player& server, const GameState& game);
// ServeType decideServeStrategy(const Player& server, const GameState& game);
// double calculateServeAdjustment(const Player& server, const GameState& game, ServeType serveType);
// int calculateServePower(const Player& server, ServeType serveType, double adjustment);
// double calculateServeFaultRate(const Player& server, ServeType serveType, double adjustment);

#endif