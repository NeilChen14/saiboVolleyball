// gameDisplay.h
#ifndef GAME_DISPLAY_H
#define GAME_DISPLAY_H

// 屏幕尺寸常量
const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 900;

#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include <functional>
#include <queue>

#include "game.h"

// UI颜色定义
struct UIColor {
    SDL_Color text = {255, 255, 255, 255};
    SDL_Color background = {30, 30, 30, 255};
    SDL_Color primary = {66, 135, 245, 255};
    SDL_Color secondary = {245, 135, 66, 255};
    SDL_Color success = {76, 175, 80, 255};
    SDL_Color error = {244, 67, 54, 255};
    SDL_Color danger = {220, 53, 69, 255};
    SDL_Color warning = {255, 193, 7, 255};
    SDL_Color border = {150, 150, 150, 255};
    SDL_Color hovered = {100, 100, 100, 180};
    SDL_Color info = {150, 200, 255, 255};
};

// 按钮结构体
struct Button {
    SDL_Rect rect;
    std::string text;
    bool hovered;
    bool clicked;
    std::function<void()> onClick;

    Button(int x = 0, int y = 0, int w = 0, int h = 0, const std::string& t = "")
        : rect{x, y, w, h}, text(t), hovered(false), clicked(false), onClick(nullptr) {}

    bool isMouseOver(int mx, int my) const {
        return mx >= rect.x && mx < rect.x + rect.w &&
               my >= rect.y && my < rect.y + rect.h;
    }
};

// 输入框结构体
struct InputBox {
    SDL_Rect rect;
    std::string text;
    bool focused;  // Changed from 'active' to 'focused' to match your code
    int maxLength;
    std::string placeholder;

    InputBox(int x = 0, int y = 0, int w = 100, int h = 30, const std::string& ph = "")
        : rect{x, y, w, h}, text(""), focused(false), maxLength(50), placeholder(ph) {}

    // Add this method to check if mouse is over the input box
    bool isMouseOver(int mouseX, int mouseY) const {
        return (mouseX >= rect.x && mouseX <= rect.x + rect.w &&
                mouseY >= rect.y && mouseY <= rect.y + rect.h);
    }
};
// UI界面枚举
enum UIScreen {
    SCREEN_MAIN_MENU,
    SCREEN_TEAM_SETUP,
    SCREEN_PLAYER_EDIT,
    SCREEN_FORMATION,
    SCREEN_GAME_RUNNING,
    SCREEN_GAME_RESULT,
    SCREEN_PAUSE_CONTINUE
};

// 比赛事件结构体
struct GameEvent {
    std::string description;
    int team;  // 0=A队, 1=B队, -1=中性
    Uint32 timestamp;

    GameEvent(const std::string& desc, int t = -1)
        : description(desc), team(t), timestamp(SDL_GetTicks()) {}
};

// 游戏显示管理器
class GameDisplay {
public:
    GameDisplay(int width, int height);
    ~GameDisplay();

    bool init();
    void handleEvent(const SDL_Event& event);
    void update();
    void render();
    bool isRunning() const { return running; }

private:
    // 初始化界面子函数
    void initMainMenu();
    void initTeamSetup();
    void initPlayerEdit();
    void initFormation();
    void initGameRunning();
    void setupGameRunningButtons();
    void initPauseContinue();

    // 事件处理子函数
    void handleMainMenuEvent(const SDL_Event& event);
    void handleTeamSetupEvent(const SDL_Event& event);
    void handlePlayerEditEvent(const SDL_Event& event);
    void handleFormationEvent(const SDL_Event& event);
    void handleGameRunningEvent(const SDL_Event& event);
    void handlePauseContinueEvent(const SDL_Event& event);

    // 渲染函数
    void renderMainMenu();
    void renderTeamSetup();
    void renderPlayerEdit();
    void renderFormation();
    void renderGameRunning();
    void renderGameResult();
    void renderGameEvents();
    void renderPauseContinue();

    // 绘图工具
    void renderButton(const Button& btn);
    void renderInputBox(const InputBox& box);
    void renderText(const std::string& text, int x, int y, TTF_Font* font, SDL_Color color);
    void renderWrappedText(const std::string& text, int x, int y, int maxWidth, TTF_Font* font, SDL_Color color);
    void renderFilledRect(int x, int y, int w, int h, SDL_Color color);
    void renderBorderedRect(int x, int y, int w, int h, SDL_Color borderColor, int borderWidth);
    SDL_Texture* createTextTexture(const std::string& text, TTF_Font* font, SDL_Color color);

    // 游戏逻辑
    void simulateRound();
    std::string intToString(int value);

    // 比赛辅助逻辑
    void ensureTeamsLoaded();
    void initMatchState();
    void nextSet();
    int currentSetTarget() const;
    void appendLog(const std::string& s);
    void appendEvent(const std::string& desc, int team = -1);
    void fixWorkingDirectoryForPlayers();

    // 暂停继续控制
    void waitForContinue(const std::string& message, std::function<void()> callback);

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* fontLarge;
    TTF_Font* fontMedium;
    TTF_Font* fontSmall;
    bool running;
    UIScreen currentScreen;

    // 简单颜色配置
    UIColor colors;

    // UI 元素
    std::vector<Button> buttons;
    std::vector<InputBox> inputBoxes;

    // 球队与游戏状态
    GameState gameState;
    int selectedTeam;
    int selectedPlayer;

    // 赛况状态
    bool autoSimulating = false;
    Uint32 lastSimTick = 0;
    Uint32 simIntervalMs = 800;  // 自动模拟间隔
    bool pauseAfterEachRally = true;  // 逐球暂停

    int setsWonA = 0;
    int setsWonB = 0;
    bool matchOver = false;

    std::vector<std::string> eventLog;
    // 比赛事件队列
    std::queue<GameEvent> gameEvents;
    int currentRallyStep = 0;  // 当前回合步骤
    std::string currentRallyDescription = "";  // 当前回合描述

    // 暂停继续控制
    bool waitingForContinue = false;
    std::function<void()> continueCallback = nullptr;
};

#endif // GAME_DISPLAY_H