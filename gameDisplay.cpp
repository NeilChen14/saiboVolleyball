// gameDisplay.cpp
#include "gameDisplay.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <mutex>

// forward declaration of internal function defined in game.cpp
extern int processRallyFromServe(GameState& game);

// === 全局 UI 日志桥接：供 game.cpp 调用 ===
namespace {
    std::mutex g_uiLogMutex;
    std::vector<std::string> g_uiLogBuffer; // 每回合详细步骤
}

void emitUIEvent(const char* msg) {
    if (!msg) return;
    std::lock_guard<std::mutex> lk(g_uiLogMutex);
    g_uiLogBuffer.emplace_back(msg);
}

namespace {
    // 本文件本地回合数，避免修改 game.h
    static int g_roundNum = 1;
}

GameDisplay::GameDisplay(int width, int height)
    : window(nullptr), renderer(nullptr), fontLarge(nullptr),
      fontMedium(nullptr), fontSmall(nullptr), running(true),
      currentScreen(SCREEN_MAIN_MENU), selectedTeam(0), selectedPlayer(0),
      waitingForContinue(false), continueCallback(nullptr) {

    // 初始化游戏状态
    gameState.setNum = 1;
    // 使用本地 round 变量，不修改 game.h
    g_roundNum = 1;
    gameState.scoreA = 0;
    gameState.scoreB = 0;

    // 初始化默认轮转（避免未初始化访问）
    for (int i = 0; i < 6; ++i) {
        gameState.rotateA[i] = i % 6;
        gameState.rotateB[i] = i % 6;
    }
}

GameDisplay::~GameDisplay() {
    if (fontLarge) TTF_CloseFont(fontLarge);
    if (fontMedium) TTF_CloseFont(fontMedium);
    if (fontSmall) TTF_CloseFont(fontSmall);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

bool GameDisplay::init() {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // 初始化TTF字体库
    if (TTF_Init() < 0) {
        std::cerr << "TTF初始化失败: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // 创建窗口
    window = SDL_CreateWindow(
        "排球比赛模拟系统",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1400, 900,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "窗口创建失败: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // 创建渲染器
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "渲染器创建失败: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // 加载字体（使用 Microsoft YaHei UI）
    fontLarge = TTF_OpenFont("C:\\Windows\\Fonts\\Microsoft YaHei UI\\msyh.ttf", 28);
    fontMedium = TTF_OpenFont("C:\\Windows\\Fonts\\Microsoft YaHei UI\\msyh.ttf", 18);
    fontSmall = TTF_OpenFont("C:\\Windows\\Fonts\\Microsoft YaHei UI\\msyh.ttf", 14);

    if (fontLarge) {
        std::cout << "成功加载 Microsoft YaHei UI (Large)" << std::endl;
    } else {
        std::cout << "Microsoft YaHei UI 加载失败，尝试其他字体" << std::endl;
    }

    // 如果加载失败，尝试其他路径
    if (!fontLarge) {
        fontLarge = TTF_OpenFont("C:\\Windows\\Fonts\\msyh.ttf", 28);
    }
    if (!fontMedium) {
        fontMedium = TTF_OpenFont("C:\\Windows\\Fonts\\msyh.ttf", 18);
    }
    if (!fontSmall) {
        fontSmall = TTF_OpenFont("C:\\Windows\\Fonts\\msyh.ttf", 14);
    }

    // 备选方案：使用黑体
    if (!fontLarge) {
        fontLarge = TTF_OpenFont("C:\\Windows\\Fonts\\SimHei.ttf", 28);
        if (fontLarge) {
            std::cout << "使用黑体 (Large)" << std::endl;
        }
    }
    if (!fontMedium) {
        fontMedium = TTF_OpenFont("C:\\Windows\\Fonts\\SimHei.ttf", 18);
    }
    if (!fontSmall) {
        fontSmall = TTF_OpenFont("C:\\Windows\\Fonts\\SimHei.ttf", 14);
    }

    if (!fontLarge || !fontMedium || !fontSmall) {
        std::cerr << "字体加载失败: fontLarge=" << (fontLarge ? "OK" : "FAIL")
                  << ", fontMedium=" << (fontMedium ? "OK" : "FAIL")
                  << ", fontSmall=" << (fontSmall ? "OK" : "FAIL") << std::endl;
        return false;
    }
    std::cout << "所有字体加载成功" << std::endl;

    // 修正工作目录，确保能找到 players.txt
    fixWorkingDirectoryForPlayers();
    // 预加载队伍数据，避免任何界面看到全0
    ensureTeamsLoaded();

    // 初始化主菜单
    initMainMenu();

    return true;
}

void GameDisplay::initMainMenu() {
    buttons.clear();
    buttons.push_back(Button(550, 250, 300, 80, "开始新游戏"));
    buttons.push_back(Button(550, 370, 300, 80, "加载游戏"));
    buttons.push_back(Button(550, 490, 300, 80, "设置"));
    buttons.push_back(Button(550, 610, 300, 80, "退出"));

    buttons[0].onClick = [this]() { currentScreen = SCREEN_TEAM_SETUP; initTeamSetup(); };
    buttons[3].onClick = [this]() { running = false; };
}

void GameDisplay::initTeamSetup() {
    buttons.clear();
    inputBoxes.clear();

    // 确保有队伍数据，避免编辑界面出现全0
    ensureTeamsLoaded();

    // A队名称输入框
    inputBoxes.push_back(InputBox(150, 150, 300, 40));
    inputBoxes.back().text = "A队";

    // B队名称输入框
    inputBoxes.push_back(InputBox(950, 150, 300, 40));
    inputBoxes.back().text = "B队";

    // 编辑A队按钮
    buttons.push_back(Button(200, 250, 200, 60, "编辑A队球员"));
    buttons[0].onClick = [this]() {
        selectedTeam = 0;
        selectedPlayer = 0;
        currentScreen = SCREEN_PLAYER_EDIT;
        initPlayerEdit();
    };

    // 编辑B队按钮
    buttons.push_back(Button(1000, 250, 200, 60, "编辑B队球员"));
    buttons[1].onClick = [this]() {
        selectedTeam = 1;
        selectedPlayer = 0;
        currentScreen = SCREEN_PLAYER_EDIT;
        initPlayerEdit();
    };

    // 进行下一步按钮
    buttons.push_back(Button(550, 700, 300, 60, "下一步 - 阵容安排"));
    buttons[2].onClick = [this]() {
        currentScreen = SCREEN_FORMATION;
        initFormation();
    };

    // 返回按钮
    buttons.push_back(Button(50, 800, 150, 60, "返回"));
    buttons[3].onClick = [this]() {
        currentScreen = SCREEN_MAIN_MENU;
        initMainMenu();
    };
}

void GameDisplay::initPlayerEdit() {
    buttons.clear();
    inputBoxes.clear();

    // 确保队伍数据已加载
    ensureTeamsLoaded();

    const Player* team = (selectedTeam == 0) ? teamA : teamB;
    Player& player = const_cast<Player&>(team[selectedPlayer]);

    // 名称
    inputBoxes.push_back(InputBox(300, 100, 200, 40));
    inputBoxes.back().text = player.name;

    // 体力（使用 player.stamina，player.h 不存在）
    inputBoxes.push_back(InputBox(300, 160, 200, 40));
    inputBoxes.back().text = intToString(player.stamina);

    // 位置选择按钮
    buttons.push_back(Button(500, 100, 150, 40, player.position));

    // 各属性输入框 — 对应 player.h 中存在的字段
    // 顺序：spike, block, serve, pass, defense, adjust
    std::vector<std::string> attributes = {"扣球", "拦网", "发球", "传球", "防守", "调整"};
    std::vector<int> values = {player.spike, player.block, player.serve, player.pass, player.defense, player.adjust};

    for (int i = 0; i < 6; i++) {
        int x = 50 + (i % 3) * 400;
        int y = 220 + (i / 3) * 80;

        inputBoxes.push_back(InputBox(x + 200, y, 80, 40));
        inputBoxes.back().text = intToString(values[i]);
    }

    // 导航按钮
    buttons.push_back(Button(100, 700, 100, 60, "上一个"));
    buttons.push_back(Button(250, 700, 100, 60, "下一个"));
    buttons.push_back(Button(550, 700, 200, 60, "保存并返回"));

    // 上一个
    buttons[buttons.size()-3].onClick = [this]() {
        if (selectedPlayer > 0) selectedPlayer--;
        initPlayerEdit();
    };

    // 下一个（team size = 7）
    buttons[buttons.size()-2].onClick = [this]() {
        if (selectedPlayer < 6) selectedPlayer++;
        initPlayerEdit();
    };

    // 保存并返回：把输入框内容写回 player，并停止文本输入
    buttons[buttons.size()-1].onClick = [this]() {
        const Player* team = (selectedTeam == 0) ? teamA : teamB;
        Player& player = const_cast<Player&>(team[selectedPlayer]);

        // 名称
        player.name = inputBoxes[0].text;

        // 体力
        try {
            player.stamina = std::stoi(inputBoxes[1].text);
            if (player.stamina < 0) player.stamina = 0;
            if (player.stamina > 100) player.stamina = 100;
        } catch (...) { /* 保持原值 */ }

        // 属性：输入框索引 2..7 对应 6 个属性
        auto parseAttr = [&](int idx, int& field) {
            if (idx < 0 || idx >= static_cast<int>(inputBoxes.size())) return;
            try {
                int v = std::stoi(inputBoxes[idx].text);
                if (v < 0) v = 0;
                if (v > 100) v = 100;
                field = v;
            } catch (...) { /* 保持原值 */ }
        };

        parseAttr(2, player.spike);
        parseAttr(3, player.block);
        parseAttr(4, player.serve);
        parseAttr(5, player.pass);
        parseAttr(6, player.defense);
        parseAttr(7, player.adjust);

        // 停止文本输入并返回上级界面
        SDL_StopTextInput();
        currentScreen = SCREEN_TEAM_SETUP;
        initTeamSetup();
    };

    // 启用 SDL 文本输入
    SDL_StartTextInput();
}

void GameDisplay::initFormation() {
    buttons.clear();

    // 为两个队选择首发阵容
    buttons.push_back(Button(200, 250, 200, 60, "A队首发配置"));
    buttons.push_back(Button(1000, 250, 200, 60, "B队首发配置"));

    buttons[0].onClick = [this]() { selectedTeam = 0; };
    buttons[1].onClick = [this]() { selectedTeam = 1; };

    // 开始比赛按钮
    buttons.push_back(Button(550, 650, 300, 80, "开始比赛"));
    buttons[2].onClick = [this]() {
        currentScreen = SCREEN_GAME_RUNNING;
        initGameRunning();
    };

    // 返回按钮
    buttons.push_back(Button(50, 800, 150, 60, "返回"));
    buttons[3].onClick = [this]() {
        currentScreen = SCREEN_TEAM_SETUP;
        initTeamSetup();
    };
}

void GameDisplay::initGameRunning() {
    // 初始化队伍与比赛状态
    ensureTeamsLoaded();
    initMatchState();
    setupGameRunningButtons();
}

void GameDisplay::setupGameRunningButtons() {
    buttons.clear();

    // 模拟一球按钮
    buttons.push_back(Button(550, 700, 300, 80, "模拟下一球"));
    buttons.back().onClick = [this]() { simulateRound(); };

    // 自动模拟按钮
    buttons.push_back(Button(250, 700, 250, 80, "自动模拟"));
    buttons.back().onClick = [this]() { autoSimulating = true; lastSimTick = SDL_GetTicks(); };

    // 暂停按钮
    buttons.push_back(Button(900, 700, 200, 80, "暂停"));
    buttons.back().onClick = [this]() { autoSimulating = false; };

    // 返回主菜单按钮
    buttons.push_back(Button(50, 800, 150, 60, "返回菜单"));
    buttons.back().onClick = [this]() {
        currentScreen = SCREEN_MAIN_MENU;
        initMainMenu();
    };

    // 重开比赛按钮
    buttons.push_back(Button(1050, 800, 150, 60, "重新开始"));
    buttons.back().onClick = [this]() {
        autoSimulating = false;
        initMatchState();
        setupGameRunningButtons();
    };
}

void GameDisplay::initPauseContinue() {
    buttons.clear();

    // 继续按钮
    buttons.push_back(Button(550, 400, 300, 80, "继续"));
    buttons[0].onClick = [this]() {
        waitingForContinue = false;
        currentScreen = SCREEN_GAME_RUNNING;
        auto cb = continueCallback;
        continueCallback = nullptr;
        // 恢复比赛界面按钮
        setupGameRunningButtons();
        if (cb) cb();
    };

    // 返回主菜单按钮
    buttons.push_back(Button(50, 800, 150, 60, "返回菜单"));
    buttons[1].onClick = [this]() {
        currentScreen = SCREEN_MAIN_MENU;
        waitingForContinue = false;
        continueCallback = nullptr;
        initMainMenu();
    };
}

void GameDisplay::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEMOTION:
        case SDL_TEXTINPUT:
        case SDL_KEYDOWN:
            switch (currentScreen) {
                case SCREEN_MAIN_MENU:
                    handleMainMenuEvent(event);
                    break;
                case SCREEN_TEAM_SETUP:
                    handleTeamSetupEvent(event);
                    break;
                case SCREEN_PLAYER_EDIT:
                    handlePlayerEditEvent(event);
                    break;
                case SCREEN_FORMATION:
                    handleFormationEvent(event);
                    break;
                case SCREEN_GAME_RUNNING:
                    handleGameRunningEvent(event);
                    break;
                case SCREEN_PAUSE_CONTINUE:
                    handlePauseContinueEvent(event);
                    break;
                default:
                    break;
            }
            break;
    }
}

void GameDisplay::handleMainMenuEvent(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int mx = event.button.x;
        int my = event.button.y;

        for (auto& btn : buttons) {
            if (btn.isMouseOver(mx, my) && btn.onClick) {
                btn.onClick();
            }
        }
    } else if (event.type == SDL_MOUSEMOTION) {
        int mx = event.motion.x;
        int my = event.motion.y;

        for (auto& btn : buttons) {
            btn.hovered = btn.isMouseOver(mx, my);
        }
    }
}

void GameDisplay::handleTeamSetupEvent(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int mx = event.button.x;
        int my = event.button.y;

        // 处理输入框焦点
        for (auto& box : inputBoxes) {
            box.focused = box.isMouseOver(mx, my);
        }

        // 处理按钮点击
        for (auto& btn : buttons) {
            if (btn.isMouseOver(mx, my) && btn.onClick) {
                btn.onClick();
            }
        }
    } else if (event.type == SDL_MOUSEMOTION) {
        int mx = event.motion.x;
        int my = event.motion.y;

        for (auto& btn : buttons) {
            btn.hovered = btn.isMouseOver(mx, my);
        }
    } else if (event.type == SDL_TEXTINPUT) {
        for (auto& box : inputBoxes) {
            if (box.focused) {
                box.text += event.text.text;
            }
        }
    } else if (event.type == SDL_KEYDOWN) {
        for (auto& box : inputBoxes) {
            if (box.focused) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && !box.text.empty()) {
                    box.text.pop_back();
                }
            }
        }
    }
}

void GameDisplay::handlePlayerEditEvent(const SDL_Event& event) {
    handleTeamSetupEvent(event);  // 复用相同的事件处理逻辑
}

void GameDisplay::handleFormationEvent(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int mx = event.button.x;
        int my = event.button.y;

        for (auto& btn : buttons) {
            if (btn.isMouseOver(mx, my) && btn.onClick) {
                btn.onClick();
            }
        }
    } else if (event.type == SDL_MOUSEMOTION) {
        int mx = event.motion.x;
        int my = event.motion.y;

        for (auto& btn : buttons) {
            btn.hovered = btn.isMouseOver(mx, my);
        }
    }
}

void GameDisplay::handleGameRunningEvent(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int mx = event.button.x;
        int my = event.button.y;

        for (auto& btn : buttons) {
            if (btn.isMouseOver(mx, my) && btn.onClick) {
                btn.onClick();
            }
        } // 处理"继续"按钮
        if (waitingForContinue) {
            Button continueBtn(SCREEN_WIDTH - 150, SCREEN_HEIGHT - 60, 130, 40, "继续");
            if (continueBtn.isMouseOver(mx, my)) {
                waitingForContinue = false;
                auto cb = continueCallback;
                continueCallback = nullptr;
                if (cb) cb();
            }
        }
    } else if (event.type == SDL_MOUSEMOTION) {
        int mx = event.motion.x;
        int my = event.motion.y;

        for (auto& btn : buttons) {
            btn.hovered = btn.isMouseOver(mx, my);
        }

        // 处理"继续"按钮悬停效果
        if (waitingForContinue) {
            Button continueBtn(SCREEN_WIDTH - 150, SCREEN_HEIGHT - 60, 130, 40, "继续");
            continueBtn.hovered = continueBtn.isMouseOver(mx, my);
        }
    }
}

void GameDisplay::handlePauseContinueEvent(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int mx = event.button.x;
        int my = event.button.y;

        for (auto& btn : buttons) {
            if (btn.isMouseOver(mx, my) && btn.onClick) {
                btn.onClick();
            }
        }
    } else if (event.type == SDL_MOUSEMOTION) {
        int mx = event.motion.x;
        int my = event.motion.y;

        for (auto& btn : buttons) {
            btn.hovered = btn.isMouseOver(mx, my);
        }
    }
}

void GameDisplay::update() {
    // 自动模拟控制
    if (currentScreen == SCREEN_GAME_RUNNING && autoSimulating && !matchOver) {
        Uint32 now = SDL_GetTicks();
        if (now - lastSimTick >= simIntervalMs) {
            simulateRound();
            lastSimTick = now;
        }
    }
}

void GameDisplay::render() {
    // 清空屏幕
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    switch (currentScreen) {
        case SCREEN_MAIN_MENU:
            renderMainMenu();
            break;
        case SCREEN_TEAM_SETUP:
            renderTeamSetup();
            break;
        case SCREEN_PLAYER_EDIT:
            renderPlayerEdit();
            break;
        case SCREEN_FORMATION:
            renderFormation();
            break;
        case SCREEN_GAME_RUNNING:
            renderGameRunning();
            break;
        case SCREEN_GAME_RESULT:
            renderGameResult();
            break;
        case SCREEN_PAUSE_CONTINUE:
            renderGameRunning(); // 先渲染游戏画面
            renderPauseContinue(); // 再渲染暂停界面
            break;
    }

    SDL_RenderPresent(renderer);
}

void GameDisplay::renderMainMenu() {
    // 标题
    renderText("排球比赛模拟系统", 450, 80, fontLarge, colors.primary);

    // 绘制主菜单按钮
    for (auto& btn : buttons) {
        renderButton(btn);
    }

    // 事件日志（右侧）
    int start = static_cast<int>(eventLog.size()) > 10 ? static_cast<int>(eventLog.size()) - 10 : 0;
    int row = 0;
    for (int i = start; i < static_cast<int>(eventLog.size()); ++i) {
        renderText(eventLog[i], 900, 250 + row * 18, fontSmall, colors.text);
        row++;
    }

    // 简要数据统计（左侧）
    renderText("A队数据", 120, 620, fontSmall, colors.primary);
    for (int i = 0; i < 7; ++i) {
        std::string line = teamA[i].name + " S:" + intToString(gameState.scoredA[i]) + " F:" + intToString(gameState.faultA[i]);
        renderText(line, 100, 650 + i * 18, fontSmall, colors.text);
    }
    renderText("B队数据", 720, 620, fontSmall, colors.secondary);
    for (int i = 0; i < 7; ++i) {
        std::string line = teamB[i].name + " S:" + intToString(gameState.scoredB[i]) + " F:" + intToString(gameState.faultB[i]);
        renderText(line, 700, 650 + i * 18, fontSmall, colors.text);
    }
}

void GameDisplay::renderTeamSetup() {
    renderText("队伍设置", 600, 30, fontLarge, colors.primary);

    // A队
    renderText("A队", 200, 120, fontMedium, colors.primary);
    if (!inputBoxes.empty()) renderInputBox(inputBoxes[0]);

    // B队
    renderText("B队", 1000, 120, fontMedium, colors.secondary);
    if (inputBoxes.size() > 1) renderInputBox(inputBoxes[1]);

    // 绘制按钮
    for (auto& btn : buttons) {
        renderButton(btn);
    }
}

void GameDisplay::renderPlayerEdit() {
    const Player* team = (selectedTeam == 0) ? teamA : teamB;
    const Player& player = team[selectedPlayer];

    std::string title = (selectedTeam == 0 ? "A队 " : "B队 ") + player.name;
    renderText(title, 50, 30, fontLarge, colors.primary);

    // 基本信息
    renderText("名称:", 100, 100, fontMedium, colors.text);
    renderInputBox(inputBoxes[0]);

    renderText("体力:", 100, 160, fontMedium, colors.text);
    renderInputBox(inputBoxes[1]);

    renderText("位置:", 400, 100, fontMedium, colors.text);
    renderButton(buttons[0]);

    // 属性编辑
    std::vector<std::string> attributes = {"扣球", "拦网", "发球", "传球", "防守", "调整"};
    for (int i = 0; i < 6; i++) {
        int x = 50 + (i % 3) * 400;
        int y = 220 + (i / 3) * 80;

        renderText(attributes[i] + ":", x, y, fontSmall, colors.text);
        renderInputBox(inputBoxes[i + 2]);
    }

    // 导航按钮
    for (size_t i = 1; i < buttons.size(); i++) {
        renderButton(buttons[i]);
    }

    // 显示当前选中信息 (team size = 7)
    std::string info = "球员 " + intToString(selectedPlayer + 1) + " / 7";
    renderText(info, 800, 700, fontSmall, colors.text);
}

void GameDisplay::renderFormation() {
    renderText("阵容安排", 600, 30, fontLarge, colors.primary);

    // 显示两个队的首发信息
    renderText("A队首发", 200, 150, fontMedium, colors.primary);
    renderText("B队首发", 1000, 150, fontMedium, colors.secondary);

    // 显示首发球员列表（避免越界）
    const int* rotateA = gameState.rotateA;
    const int* rotateB = gameState.rotateB;

    for (int i = 0; i < 6; i++) {
        int idxA = std::clamp(rotateA[i], 0, 6);
        int idxB = std::clamp(rotateB[i], 0, 6);
        std::string playerA = teamA[idxA].name;
        std::string playerB = teamB[idxB].name;

        renderText(playerA, 100, 250 + i * 60, fontSmall, colors.text);
        renderText(playerB, 900, 250 + i * 60, fontSmall, colors.text);
    }

    // 绘制按钮
    for (auto& btn : buttons) {
        renderButton(btn);
    }
}

void GameDisplay::renderGameRunning() {
    // 比分显示
    std::string scoreStr = "A队 " + intToString(gameState.scoreA) +
                          " : " + intToString(gameState.scoreB) + " B队";
    renderText(scoreStr, 500, 50, fontLarge, colors.primary);

    // 局数和回合数（使用本地 g_roundNum）
    std::string roundStr = "第" + intToString(gameState.setNum) + "局 第" +
                          intToString(g_roundNum) + "回合";
    renderText(roundStr, 550, 150, fontMedium, colors.text);

    // 局分与发球方
    std::string setsStr = "局分 A " + intToString(setsWonA) + " - " + intToString(setsWonB) + " B";
    renderText(setsStr, 520, 180, fontSmall, colors.text);
    std::string serveStr = std::string("发球方: ") + (gameState.serveSide == 0 ? "A队" : "B队");
    renderText(serveStr, 560, 210, fontSmall, colors.text);
    renderText(autoSimulating ? "自动模拟: 开" : "自动模拟: 关", 560, 240, fontSmall, colors.text);

    // 球场显示
    renderBorderedRect(100, 250, 1200, 350, colors.border, 3);

    // A队阵容
    const int* rotateA = gameState.rotateA;
    for (int i = 0; i < 6; i++) {
        int x = 200 + (i % 3) * 300;
        int y = 280 + (i / 3) * 150;
        int idx = std::clamp(rotateA[i], 0, 6);
        renderText(teamA[idx].name, x, y, fontSmall, colors.primary);
    }

    // B队阵容
    const int* rotateB = gameState.rotateB;
    for (int i = 0; i < 6; i++) {
        int x = 200 + (i % 3) * 300;
        int y = 450 + (i / 3) * 150;
        int idx = std::clamp(rotateB[i], 0, 6);
        renderText(teamB[idx].name, x, y, fontSmall, colors.secondary);
    }

    // 绘制按钮
    for (auto& btn : buttons) {
        renderButton(btn);
    }
    // 如果正在等待继续，渲染"继续"按钮
    if (waitingForContinue) {
        Button continueBtn(SCREEN_WIDTH - 150, SCREEN_HEIGHT - 60, 130, 40, "继续");
        renderButton(continueBtn);
    }

    // 渲染比赛事件
    renderGameEvents();
}

void GameDisplay::renderGameResult() {
    renderText("比赛结果", 600, 200, fontLarge, colors.primary);

    std::string winner = (setsWonA > setsWonB) ? "A队胜" : "B队胜";
    renderText(winner, 550, 400, fontLarge, colors.success);

    std::string finalScore = intToString(gameState.scoreA) + " : " + intToString(gameState.scoreB);
    renderText(finalScore, 550, 500, fontMedium, colors.text);

    std::string setSummary = "局分 A " + intToString(setsWonA) + " - " + intToString(setsWonB) + " B";
    renderText(setSummary, 520, 460, fontMedium, colors.text);

    // 显示详细统计数据
    renderText("A队统计数据:", 200, 550, fontMedium, colors.primary);
    for (int i = 0; i < 7; i++) {
        std::string line = teamA[i].name + " 得分:" + intToString(gameState.scoredA[i]) +
                          " 失误:" + intToString(gameState.faultA[i]);
        renderText(line, 200, 580 + i * 25, fontSmall, colors.text);
    }

    renderText("B队统计数据:", 800, 550, fontMedium, colors.secondary);
    for (int i = 0; i < 7; i++) {
        std::string line = teamB[i].name + " 得分:" + intToString(gameState.scoredB[i]) +
                          " 失误:" + intToString(gameState.faultB[i]);
        renderText(line, 800, 580 + i * 25, fontSmall, colors.text);
    }
}

void GameDisplay::renderButton(const Button& btn) {
    SDL_Color bgColor = btn.hovered ? colors.hovered : colors.primary;
    renderFilledRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, bgColor);
    renderBorderedRect(btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h, colors.border, 2);

    renderText(btn.text,
               btn.rect.x + (btn.rect.w - static_cast<int>(btn.text.length()) * 8) / 2,
               btn.rect.y + (btn.rect.h - 18) / 2,
               fontMedium, colors.text);
}

void GameDisplay::renderInputBox(const InputBox& box) {
    SDL_Color bgColor = box.focused ? SDL_Color{50, 50, 100, 255} : SDL_Color{50, 50, 50, 255};
    renderFilledRect(box.rect.x, box.rect.y, box.rect.w, box.rect.h, bgColor);
    renderBorderedRect(box.rect.x, box.rect.y, box.rect.w, box.rect.h, colors.primary, 2);

    renderText(box.text, box.rect.x + 10, box.rect.y + 10, fontSmall, colors.text);
}

void GameDisplay::renderText(const std::string& text, int x, int y, TTF_Font* font, SDL_Color color) {
    if (!font) return;

    // 使用 UTF-8 渲染以支持中文
    SDL_Surface* surface = TTF_RenderUTF8_Solid(font, text.c_str(), color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect rect = {x, y, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, nullptr, &rect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    } else {
        std::cerr << "文本渲染失败: " << TTF_GetError() << std::endl;
    }
}

void GameDisplay::renderWrappedText(const std::string& text, int x, int y, int maxWidth, TTF_Font* font, SDL_Color color) {
    if (!font) return;

    std::string line = "";
    int lineHeight = TTF_FontHeight(font);
    int currentY = y;

    for (size_t i = 0; i < text.length(); i++) {
        if (text[i] == '\n') {
            renderText(line, x, currentY, font, color);
            line = "";
            currentY += lineHeight;
            continue;
        }

        line += text[i];

        // 检查是否超出宽度
        int textWidth;
        TTF_SizeUTF8(font, line.c_str(), &textWidth, nullptr);

        if (textWidth > maxWidth) {
            // 找到最后一个空格位置回退
            size_t lastSpace = line.find_last_of(' ');
            if (lastSpace != std::string::npos && lastSpace > 0) {
                std::string toRender = line.substr(0, lastSpace);
                renderText(toRender, x, currentY, font, color);
                line = line.substr(lastSpace + 1);
            } else {
                // 单词太长，强制换行
                renderText(line, x, currentY, font, color);
                line = "";
            }
            currentY += lineHeight;
        }
    }

    // 渲染最后一行
    if (!line.empty()) {
        renderText(line, x, currentY, font, color);
    }
}

void GameDisplay::renderFilledRect(int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void GameDisplay::renderBorderedRect(int x, int y, int w, int h, SDL_Color borderColor, int borderWidth) {
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_Rect rect = {x, y, w, h};
    for (int i = 0; i < borderWidth; i++) {
        SDL_Rect borderRect = {x + i, y + i, w - 2*i, h - 2*i};
        SDL_RenderDrawRect(renderer, &borderRect);
    }
}

SDL_Texture* GameDisplay::createTextTexture(const std::string& text, TTF_Font* font, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderUTF8_Solid(font, text.c_str(), color);
    if (!surface) return nullptr;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void GameDisplay::simulateRound() {
    if (matchOver) return;

    // 清空之前的比赛事件
    while (!gameEvents.empty()) gameEvents.pop();
    currentRallyStep = 0;
    currentRallyDescription = "";

    // 当前发球方与发球人
    Player server = (gameState.serveSide == 0)
        ? teamA[gameState.rotateA[0]]
        : teamB[gameState.rotateB[0]];

    appendLog(std::string("第") + intToString(gameState.setNum) + "局 第" + intToString(g_roundNum) + "球 - 发球: " +
              (gameState.serveSide == 0 ? std::string("A ") + server.name : std::string("B ") + server.name));

    // 使用真实比赛回合逻辑
    int scorer = processRallyFromServe(gameState); // 0=A, 1=B

    // 将底层详细事件导入到UI事件面板
    {
        std::vector<std::string> copied;
        {
            std::lock_guard<std::mutex> lk(g_uiLogMutex);
            copied.swap(g_uiLogBuffer);
        }
        std::cout << "Copied " << copied.size() << " events" << std::endl; // 添加调试信息
        for (const auto& ev : copied) {
            std::cout << "Event: " << ev << std::endl; // 添加调试信息
            appendEvent(ev);
        }
    }

    if (scorer == 0) {
        gameState.scoreA++;
        appendLog("A队得分");
        appendEvent("A队得分", 0);
        if (gameState.serveSide == 0) {
            // 发球方连得分，不轮转
        } else {
            // 换发与轮转到A
            rotateTeam(gameState, 0);
            gameState.serveSide = 0;
            // 若失分方的发球人是MB，进入自由人
            if (server.position == "MB") {
                gameState.liberoReplaceB = gameState.rotateB[0];
                gameState.rotateB[0] = 6;
            }
        }
    } else {
        gameState.scoreB++;
        appendLog("B队得分");
        appendEvent("B队得分", 1);
        if (gameState.serveSide == 1) {
            // 发球方连得分，不轮转
        } else {
            // 换发与轮转到B
            rotateTeam(gameState, 1);
            gameState.serveSide = 1;
            if (server.position == "MB") {
                gameState.liberoReplaceA = gameState.rotateA[0];
                gameState.rotateA[0] = 6;
            }
        }
    }

    // 回合数+1
    g_roundNum++;

    // 附加当前比分到日志
    appendLog(std::string("当前比分 A:") + intToString(gameState.scoreA) + " - B:" + intToString(gameState.scoreB));

    // 判定本局结束
    int target = currentSetTarget();
    if ((gameState.scoreA >= target || gameState.scoreB >= target) && std::abs(gameState.scoreA - gameState.scoreB) >= 2) {
        if (gameState.scoreA > gameState.scoreB) {
            setsWonA++;
            appendLog("本局A队胜");
            appendEvent("本局A队胜", 0);
        } else {
            setsWonB++;
            appendLog("本局B队胜");
            appendEvent("本局B队胜", 1);
        }

        if (setsWonA == 2 || setsWonB == 2 || gameState.setNum >= 3) {
            autoSimulating = false;
            matchOver = true;
            currentScreen = SCREEN_GAME_RESULT;
        } else {
            nextSet();
        }
    }

    // 若需要逐球暂停，弹出“继续”覆盖层（自动模拟关闭时）
    if (!matchOver && pauseAfterEachRally && !autoSimulating) {
        waitForContinue("回合结束，点击继续", [this]() {
            // 点击“继续”后直接打下一球
            simulateRound();
        });
    }
}

void GameDisplay::waitForContinue(const std::string& message, std::function<void()> callback) {
    waitingForContinue = true;
    continueCallback = callback;
    // 显示消息
    appendEvent(message);
}

// 辅助：确保队伍数据已加载
void GameDisplay::ensureTeamsLoaded() {
    // 若已有名字，则视为已加载
    if (!teamA[0].name.empty() && !teamB[0].name.empty()) return;

    // 尝试读取players.txt
    readData();

    // 检查是否成功读取了足够的球员数据
    if (allPlayers.size() >= 15) {
        inputPlayerByPreset();
        appendLog("已从 players.txt 载入预设队伍");
    } else {
        // 回退：构造默认队伍
        appendLog("未找到有效的 players.txt，使用默认队伍");
        auto makeP = [](const std::string& n, const std::string& pos) {
            Player p{};
            p.name = n; p.position = pos; p.gender = 1;
            p.spike = p.block = p.serve = p.pass = p.defense = p.adjust = 60;
            p.stamina = 80;
            p.mental = {60, 60, 60, 60, 40};
            p.wisdom = 60;
            return p;
        };

        Player ta[7] = {
            makeP("A1", "OH"), makeP("A2", "S"), makeP("A3", "MB"),
            makeP("A4", "OH"), makeP("A5", "MB"), makeP("A6", "OP"),
            makeP("AL", "L")
        };
        Player tb[7] = {
            makeP("B1", "OH"), makeP("B2", "S"), makeP("B3", "MB"),
            makeP("B4", "OH"), makeP("B5", "MB"), makeP("B6", "OP"),
            makeP("BL", "L")
        };
        for (int i = 0; i < 7; ++i) { teamA[i] = ta[i]; teamB[i] = tb[i]; }
    }

    // 规范化位置字符串，兼容中文与缩写
    auto normalize = [](std::string& pos) {
        if (pos == "主攻" || pos == "OH") pos = "OH";
        else if (pos == "副攻" || pos == "MB") pos = "MB";
        else if (pos == "二传" || pos == "S") pos = "S";
        else if (pos == "自由人" || pos == "L") pos = "L";
        else if (pos == "接应" || pos == "OP") pos = "OP";
    };
    for (int i = 0; i < 7; ++i) {
        normalize(teamA[i].position);
        normalize(teamB[i].position);
    }

    // 最终校验：若仍为空名或关键能力为0，填充默认值，避免UI显示0
    auto fixIfEmpty = [](Player& p, const std::string& fallbackName, const std::string& fallbackPos){
        if (p.name.empty()) p.name = fallbackName;
        if (p.position.empty()) p.position = fallbackPos;
        if (p.spike == 0 && p.block == 0 && p.serve == 0 && p.pass == 0 && p.defense == 0) {
            p.spike = p.block = p.serve = p.pass = p.defense = 60; p.adjust = 60;
            p.stamina = 80; p.mental = {60,60,60,60,40}; p.wisdom = 60;
        }
    };
    const char* posOrder[7] = {"OH","S","MB","OH","MB","OP","L"};
    for (int i = 0; i < 7; ++i) { fixIfEmpty(teamA[i], std::string("A")+char('1'+i), posOrder[i]); }
    for (int i = 0; i < 7; ++i) { fixIfEmpty(teamB[i], std::string("B")+char('1'+i), posOrder[i]); }
}

void GameDisplay::initMatchState() {
    matchOver = false;
    autoSimulating = false;
    setsWonA = setsWonB = 0;
    eventLog.clear();
    while (!gameEvents.empty()) gameEvents.pop();
    g_roundNum = 1;

    static bool seeded = false;
    if (!seeded) { std::srand(static_cast<unsigned int>(std::time(nullptr))); seeded = true; }

    gameState.serveSide = std::rand() % 2;
    gameState.setNum = 1;
    gameState.scoreA = 0; gameState.scoreB = 0;
    for (int i = 0; i < 6; ++i) { gameState.rotateA[i] = i; gameState.rotateB[i] = i; }

    // 初始化自由人替换（参考 newGame()）
    if (teamA[gameState.rotateA[5]].position == "MB") { gameState.liberoReplaceA = gameState.rotateA[5]; gameState.rotateA[5] = 6; }
    if (teamA[gameState.rotateA[4]].position == "MB") { gameState.liberoReplaceA = gameState.rotateA[4]; gameState.rotateA[4] = 6; }
    if (teamA[gameState.rotateA[0]].position == "MB" && gameState.serveSide != 0) { gameState.liberoReplaceA = gameState.rotateA[0]; gameState.rotateA[0] = 6; }

    if (teamB[gameState.rotateB[5]].position == "MB") { gameState.liberoReplaceB = gameState.rotateB[5]; gameState.rotateB[5] = 6; }
    if (teamB[gameState.rotateB[4]].position == "MB") { gameState.liberoReplaceB = gameState.rotateB[4]; gameState.rotateB[4] = 6; }
    if (teamB[gameState.rotateB[0]].position == "MB" && gameState.serveSide != 1) { gameState.liberoReplaceB = gameState.rotateB[0]; gameState.rotateB[0] = 6; }

    appendLog(std::string("比赛开始！首发发球方：") + (gameState.serveSide == 0 ? "A队" : "B队"));
    appendEvent(std::string("比赛开始！首发发球方：") + (gameState.serveSide == 0 ? "A队" : "B队"));
}

void GameDisplay::nextSet() {
    // 重置比分与回合
    gameState.scoreA = gameState.scoreB = 0;
    g_roundNum = 1;

    if (gameState.setNum == 1) {
        gameState.setNum = 2;
        gameState.serveSide = 1 - gameState.serveSide; // 第二局交换发球权
    } else {
        gameState.setNum = 3;
        gameState.serveSide = std::rand() % 2; // 第三局随机
    }

    for (int i = 0; i < 6; ++i) { gameState.rotateA[i] = i; gameState.rotateB[i] = i; }

    if (teamA[gameState.rotateA[5]].position == "MB") { gameState.liberoReplaceA = gameState.rotateA[5]; gameState.rotateA[5] = 6; }
    if (teamA[gameState.rotateA[4]].position == "MB") { gameState.liberoReplaceA = gameState.rotateA[4]; gameState.rotateA[4] = 6; }
    if (teamA[gameState.rotateA[0]].position == "MB" && gameState.serveSide != 0) { gameState.liberoReplaceA = gameState.rotateA[0]; gameState.rotateA[0] = 6; }

    if (teamB[gameState.rotateB[5]].position == "MB") { gameState.liberoReplaceB = gameState.rotateB[5]; gameState.rotateB[5] = 6; }
    if (teamB[gameState.rotateB[4]].position == "MB") { gameState.liberoReplaceB = gameState.rotateB[4]; gameState.rotateB[4] = 6; }
    if (teamB[gameState.rotateB[0]].position == "MB" && gameState.serveSide != 1) { gameState.liberoReplaceB = gameState.rotateB[0]; gameState.rotateB[0] = 6; }

    appendLog(std::string("开始第") + intToString(gameState.setNum) + "局，发球方：" + (gameState.serveSide == 0 ? "A队" : "B队"));
    appendEvent(std::string("开始第") + intToString(gameState.setNum) + "局，发球方：" + (gameState.serveSide == 0 ? "A队" : "B队"));
}

int GameDisplay::currentSetTarget() const {
    return (gameState.setNum == 3) ? 15 : 25;
}

void GameDisplay::appendLog(const std::string& s) {
    eventLog.push_back(s);
    if (eventLog.size() > 50) {
        eventLog.erase(eventLog.begin(), eventLog.begin() + (eventLog.size() - 50));
    }
}

void GameDisplay::appendEvent(const std::string& desc, int team) {
    gameEvents.push(GameEvent(desc, team));
    // 限制事件队列大小
    if (gameEvents.size() > 50) {
        gameEvents.pop();
    }
}

void GameDisplay::renderGameEvents() {
    // 渲染比赛事件区域背景 - 扩大并调整位置
    int eventAreaY = 330;
    int eventAreaHeight = SCREEN_HEIGHT - eventAreaY - 10;
    renderFilledRect(10, eventAreaY, SCREEN_WIDTH - 20, eventAreaHeight, SDL_Color{30, 30, 30, 230});
    renderBorderedRect(10, eventAreaY, SCREEN_WIDTH - 20, eventAreaHeight, colors.border, 2);

    // 标题和状态信息
    renderText("比赛进程", 20, eventAreaY + 5, fontMedium, colors.info);

    // 显示当前回合的详细动作
    if (!currentRallyDescription.empty()) {
        std::istringstream iss(currentRallyDescription);
        std::string line;
        int lineY = eventAreaY + 30;
        int maxWidth = SCREEN_WIDTH - 40;

        while (std::getline(iss, line, '\n')) {
            // 解析动作类型并着色
            SDL_Color lineColor = colors.text;
            if (line.find("发球") != std::string::npos) {
                lineColor = colors.success; // 发球用绿色
            } else if (line.find("扣球") != std::string::npos) {
                lineColor = colors.danger;  // 扣球用红色
            } else if (line.find("拦网") != std::string::npos) {
                lineColor = colors.warning; // 拦网用黄色
            } else if (line.find("A队") != std::string::npos) {
                lineColor = colors.primary; // A队用蓝色
            } else if (line.find("B队") != std::string::npos) {
                lineColor = colors.secondary; // B队用橙色
            }

            // 自动换行处理
            renderWrappedText(line, 20, lineY, maxWidth, fontSmall, lineColor);
            lineY += 20;
        }
    }

    // 显示历史事件（最近3条）
    if (!gameEvents.empty()) {
        renderText("--- 历史记录 ---", 20, eventAreaY + 150, fontSmall, colors.text);

        std::queue<GameEvent> tempQueue = gameEvents;
        int yPos = eventAreaY + 180;
        int count = 0;

        while (!tempQueue.empty() && count < 3) {
            GameEvent event = tempQueue.front();
            tempQueue.pop();

            SDL_Color color = colors.text;
            if (event.team == 0) color = colors.primary;
            else if (event.team == 1) color = colors.secondary;

            renderWrappedText(event.description, 20, yPos, SCREEN_WIDTH - 40, fontSmall, color);
            yPos += 30;
            count++;
        }
    }
}

void GameDisplay::renderPauseContinue() {
    // 渲染半透明覆盖层
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_Rect overlay = {0, 0, 1400, 900};
    SDL_RenderFillRect(renderer, &overlay);

    // 显示提示信息
    renderText("比赛暂停", 600, 200, fontLarge, colors.primary);
    renderText("点击'继续'按钮继续比赛", 500, 300, fontMedium, colors.text);

    // 绘制按钮
    for (auto& btn : buttons) {
        renderButton(btn);
    }
}

void GameDisplay::fixWorkingDirectoryForPlayers() {
    namespace fs = std::filesystem;
    auto hasPlayers = [](const fs::path& p) {
        std::error_code ec;
        return fs::exists(p / "players.txt", ec);
    };

    // 1) 如果当前目录已有，直接使用
    try {
        fs::path cur = fs::current_path();
        if (hasPlayers(cur)) {
            appendLog(std::string("使用工作目录：") + cur.string());
            return;
        }

        // 2) 检查可执行文件所在目录
        char* base = SDL_GetBasePath();
        if (base) {
            fs::path execPath(base);
            SDL_free(base);
            if (hasPlayers(execPath)) {
                std::error_code ec;
                fs::current_path(execPath, ec);
                appendLog(std::string("设置工作目录为：") + execPath.string());
                return;
            }
        }

        // 3) 检查上级目录
        fs::path parent = cur.parent_path();
        if (hasPlayers(parent)) {
            std::error_code ec;
            fs::current_path(parent, ec);
            appendLog(std::string("设置工作目录为：") + parent.string());
            return;
        }

        // 4) 检查源代码目录（相对路径）
        fs::path sourcePath = cur / "saiboVolleyball";
        if (hasPlayers(sourcePath)) {
            std::error_code ec;
            fs::current_path(sourcePath, ec);
            appendLog(std::string("设置工作目录为：") + sourcePath.string());
            return;
        }

    } catch (...) {
        appendLog("工作目录设置失败");
    }
}

std::string GameDisplay::intToString(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}
