#include "game.h"
#include "serve.h"
#include "receiveServe.h"
#include "setBall.h"
#include "spike.h"
#include "block.h"
#include "defense.h"
#include "config.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <sstream>

// 在 game.cpp 文件开头添加
extern void emitUIEvent(const char* msg);

int processRallyFromReceive(GameState& game, int attackingTeam, int defendingTeam, const ReceiveResult& receiveResult);

void rotateTeam(GameState& game, int teamID) {
    if(teamID == 0) {// A队
        int temp = game.rotateA[0];  // 原1号位球员
        for(int i = 0; i < 5; i++) {
            game.rotateA[i] = game.rotateA[i + 1];  // 2-6号位前移
        }
        game.rotateA[5] = temp;  // 原1号位到6号位

        if(teamA[game.rotateA[3]].position == "L") {//自由人即将换到4号位
            game.rotateA[3] = game.liberoReplaceA;
        }
    } else {
        int temp = game.rotateB[0];  // 原1号位球员
        for(int i = 0; i < 5; i++) {
            game.rotateB[i] = game.rotateB[i + 1];  // 2-6号位前移
        }
        game.rotateB[5] = temp;  // 原1号位到6号位

        if(teamB[game.rotateB[3]].position == "L") {//自由人即将换到4号位
            game.rotateB[3] = game.liberoReplaceB;
        }
    }
}

// 辅助函数：将防守结果转换为接一结果
ReceiveResult convertDefenseToReceive(const DefenseResult& defenseResult) {
    ReceiveResult receiveResult;

    // 根据防守质量设置接一质量
    switch(defenseResult.quality) {
        case DEFENSE_PERFECT:
            receiveResult.quality = RECEIVE_PERFECT;
            receiveResult.qualityValue = defenseResult.qualityValue;
            receiveResult.description = "完美防守，可以组织快攻";
            break;
        case DEFENSE_GOOD:
            receiveResult.quality = RECEIVE_GOOD;
            receiveResult.qualityValue = defenseResult.qualityValue;
            receiveResult.description = "好防守，可以组织进攻";
            break;
        case DEFENSE_BAD:
            receiveResult.quality = RECEIVE_BAD;
            receiveResult.qualityValue = defenseResult.qualityValue;
            receiveResult.description = "防守不到位，只能调整攻";
            break;
        case DEFENSE_FAULT:
            receiveResult.quality = RECEIVE_FAULT;
            receiveResult.qualityValue = defenseResult.qualityValue;
            receiveResult.description = "防守失误";
            break;
    }

    receiveResult.receiver = defenseResult.defender;
    // position和positionIndex需要根据实际情况设置
    // 这里暂时设为-1，表示未知
    receiveResult.position = -1;

    return receiveResult;
}

// 辅助函数：将拦回球转换为扣球结果
SpikeResult convertBlockBackToSpike(const BlockResultInfo& blockResult, const Player& attacker) {
    SpikeResult spikeResult;

    spikeResult.attacker = attacker;
    spikeResult.strategy = STRONG_ATTACK; // 拦回球类似强攻
    spikeResult.spikePower = blockResult.blockBackPower;
    spikeResult.blockCoefficient = 1.0; // 标准拦网系数
    spikeResult.isError = false;
    spikeResult.isOut = false;
    spikeResult.description = "拦回球";
    spikeResult.isSetterDump = false;

    return spikeResult;
}

// 处理一次完整的攻防回合（从发球开始）
int processRallyFromServe(GameState& game) {
    int attackingTeam = 1 - game.serveSide; // 接发球方开始进攻
    int defendingTeam = game.serveSide;     // 发球方开始防守

    // 1. 发球
    Player server;
    if(game.serveSide == 0) {
        server = teamA[game.rotateA[0]];  // A队1号位发球
    } else {
        server = teamB[game.rotateB[0]];  // B队1号位发球
    }

    Serve serve(server, game);
    ServeResult serveResult = serve.simulate();



    std::string serveTypeStr = (serveResult.type == STABLE_SERVE) ? "稳定发球" : "冲发球";
    char buffer[256];
    sprintf(buffer, "%s队%s使用%s...",
            (game.serveSide == 0) ? "A" : "B",
            server.name.c_str(),
            serveTypeStr.c_str());
    emitUIEvent(buffer);


    if (!serveResult.success) {
        // 发球失误
        emitUIEvent("发球失误！");

        if(game.serveSide == 0) {//失误统计
            game.faultA[game.rotateA[0]]++;
        } else {
            game.faultB[game.rotateB[0]]++;
        }

        return attackingTeam; // 防守方（发球方）失分，进攻方得分
    }


    sprintf(buffer, "发球成功，效果值：%d", serveResult.effectiveness);
    emitUIEvent(buffer);

    #if PAUSE_FOR_READ
    system("pause");
    #endif

    // 2. 接一

    ReceiveServe receiveServe(game, attackingTeam, serveResult.effectiveness);
    ReceiveResult receiveResult = receiveServe.simulate();

    // 显示接一阵型信息
    ReceiveFormation formation = receiveServe.getReceiveFormation();
    std::string formationStr = (formation == FORMATION_4_PLAYER) ? "4人接一" : "3人接一";

    sprintf(buffer, "%s队采用%s阵型", (attackingTeam == 0) ? "A" : "B", formationStr.c_str());
    emitUIEvent(buffer);

    // 显示接一结果

    sprintf(buffer, "%s队%s接一：%s（质量值：%d）",
            (attackingTeam == 0) ? "A" : "B",
            receiveResult.receiver.name.c_str(),
            receiveResult.description.c_str(),
            receiveResult.qualityValue);
    emitUIEvent(buffer);


    if (receiveResult.quality == RECEIVE_FAULT) {
        // 接飞
        emitUIEvent("接飞！直接失分");
        if(game.serveSide == 0) {//得分统计，ace球
            game.scoredA[game.rotateA[0]]++;
        } else {
            game.scoredB[game.rotateB[0]]++;
        }

        return defendingTeam; // 发球方得分
    }

    #if PAUSE_FOR_READ
        system("pause");
    #endif

    // 进入主循环
    return processRallyFromReceive(game, attackingTeam, defendingTeam, receiveResult);
}

// 处理一次完整的攻防回合（从接一/防守成功开始）
int processRallyFromReceive(GameState& game, int attackingTeam, int defendingTeam, const ReceiveResult& receiveResult) {
    // 保存当前的攻防状态，用于循环
    int currentAttackingTeam = attackingTeam;
    int currentDefendingTeam = defendingTeam;

    // 当前轮次的接一结果
    ReceiveResult currentReceiveResult = receiveResult;

    // 循环计数器，防止无限循环
    int rallyCount = 0;

    while (rallyCount < MAX_RALLY_COUNT) {
        rallyCount++;

        // 3. 二传
        const int* rotation = (currentAttackingTeam == 0) ? game.rotateA : game.rotateB;
        const Player* team = (currentAttackingTeam == 0) ? teamA : teamB;

        Player setter;
        int setter_id = -1;
        for (int i = 0; i < 6; i++) {
            if (team[rotation[i]].position == "S" || team[rotation[i]].position == "二传") {
                setter = team[rotation[i]];
                setter_id = rotation[i];
                break;
            }
        }

        Setter setterObj(setter, game, currentAttackingTeam);
        PassResult passResult = setterObj.simulateSet(currentReceiveResult);

        // 显示传球结果
        std::string targetStr;
        switch (passResult.target) {
            case FRONT_SPIKER:
                targetStr = "前排主攻";
                break;
            case FRONT_BLOCKER:
                targetStr = "前排副攻";
                break;
            case BACK_SPIKER:
                targetStr = "后排主攻";
                break;
            case OPPOSITE:
                targetStr = "接应";
                break;
            case SETTER_DUMP:
                targetStr = "二传二次进攻";
                break;
            case ADJUST_ATTACK:
                targetStr = "调整攻";
                break;
        }

        if (passResult.isSetterDump) {
            // 二次进攻的特殊显示格式
            char buffer[256];
            sprintf(buffer, "%s队%s二次进攻：%s（质量值：%d）",
                    (currentAttackingTeam == 0) ? "A" : "B",
                    setter.name.c_str(),
                    passResult.description.c_str(),
                    passResult.qualityValue);
            emitUIEvent(buffer);

        } else {
            // 正常传球的显示格式
            char buffer[256];
            sprintf(buffer, "%s队%s传球给%s：%s（质量值：%d）",
                    (currentAttackingTeam == 0) ? "A" : "B",
                    setter.name.c_str(),
                    passResult.targetPlayer.name.c_str(),
                    passResult.description.c_str(),
                    passResult.qualityValue);
            emitUIEvent(buffer);

        }

        if (passResult.isSetterDump) {
            char buffer[256];
            sprintf(buffer, "二次进攻效果值：%d", passResult.dumpEffectiveness);
            emitUIEvent(buffer);

        }

        // 如果二传失误，直接失分
        if (passResult.quality == POOR_PASS && !passResult.isSetterDump) {
            emitUIEvent("传球失误！直接失分");


            if(currentAttackingTeam == 0) {
                game.faultA[setter_id]++;
            } else {
                game.faultB[setter_id]++;
            }

            return currentDefendingTeam; // 防守方得分
        }

        #if PAUSE_FOR_READ
                system("pause");
        #endif

        // 4. 扣球
        SpikeResult spikeResult;

        int attackerID = 0;


        if (passResult.isSetterDump) {
            // 二次进攻
            char buffer[256];
            sprintf(buffer, "%s进行二次进攻...", setter.name.c_str());
            emitUIEvent(buffer);


            spikeResult = Spiker::createSetterDumpResult(setter, passResult.dumpEffectiveness);

            // 显示二次进攻结果

            sprintf(buffer, "%s使用二次进攻：%s",
                    spikeResult.attacker.name.c_str(),
                    spikeResult.description.c_str());
            emitUIEvent(buffer);


            if (spikeResult.isError) {
                emitUIEvent("二次进攻失误！失分");


                if(currentAttackingTeam == 0) {
                    game.faultA[setter_id]++;
                } else {
                    game.faultB[setter_id]++;
                }

                return currentDefendingTeam; // 防守方得分
            }


            sprintf(buffer, "二次进攻强度：%d，拦网系数：%.2f",
                    spikeResult.spikePower,
                    spikeResult.blockCoefficient);
            emitUIEvent(buffer);

        } else {
            // 正常扣球
            char buffer[256];
            sprintf(buffer, "%s准备扣球...", passResult.targetPlayer.name.c_str());
            emitUIEvent(buffer);


            Spiker spiker(passResult.targetPlayer, game, currentAttackingTeam);
            spikeResult = spiker.simulateSpike(passResult);

            for(int i = 0; i < 6; i++) {
                if(currentAttackingTeam == 0) {
                    if(spikeResult.attacker.name == teamA[rotation[i]].name) {
                        attackerID = rotation[i];
                        break;
                    }
                } else {
                    if(spikeResult.attacker.name == teamB[rotation[i]].name) {
                        attackerID = rotation[i];
                        break;
                    }
                }
            }

            // 显示扣球策略
            std::string strategyStr;
            switch (spikeResult.strategy) {
                case STRONG_ATTACK:
                    strategyStr = "强攻";
                    break;
                case AVOID_BLOCK:
                    strategyStr = "避手";
                    break;
                case DROP_SHOT:
                    strategyStr = "吊球";
                    break;
                case QUICK_ATTACK:
                    strategyStr = "快球";
                    break;
                case ADJUST_SPIKE:
                    strategyStr = "调整攻";
                    break;
                case TRANSITION_ATTACK:
                    strategyStr = "过渡";
                    break;
                case SETTER_SPIKE:
                    strategyStr = "二次进攻";
                    break;
            }


            sprintf(buffer, "%s使用%s：%s",
                    spikeResult.attacker.name.c_str(),
                    strategyStr.c_str(),
                    spikeResult.description.c_str());
            emitUIEvent(buffer);




            if (spikeResult.isError) {
                emitUIEvent("扣球失误！失分");


                if(currentAttackingTeam == 0) {
                    game.faultA[attackerID]++;
                } else {
                    game.faultB[attackerID]++;
                }

                return currentDefendingTeam; // 防守方得分
            }


            sprintf(buffer, "扣球强度：%d，拦网系数：%.2f",
                    spikeResult.spikePower,
                    spikeResult.blockCoefficient);
            emitUIEvent(buffer);

        }

        #if PAUSE_FOR_READ
                system("pause");
        #endif

        // 5. 拦网
        Blocker blocker(game, currentDefendingTeam, currentAttackingTeam);
        BlockResultInfo blockResult = blocker.simulateBlock(spikeResult);

        // 显示拦网结果
        char buffer[256];
        sprintf(buffer, "%s队拦网：%s（拦网强度：%d，效果值：%.2f）",
                (currentDefendingTeam == 0) ? "A" : "B",
                blockResult.description.c_str(),
                blockResult.blockPower,
                blockResult.blockEffect);
        emitUIEvent(buffer);


        // 根据拦网结果处理
        switch (blockResult.result) {
            case BLOCK_BACK: {
                // 拦回，防守方需要防守拦回球
                char buffer[256];
                sprintf(buffer, "球被拦回！%s队需要防守拦回球", (currentAttackingTeam == 0) ? "A" : "B");
                emitUIEvent(buffer);

                // 交换攻防角色：原进攻方现在防守拦回球
                std::swap(currentAttackingTeam, currentDefendingTeam);

                // 6. 防守拦回球
                Defender defender(game, currentDefendingTeam, currentAttackingTeam);
                DefenseResult defenseResult = defender.simulateDefenseAgainstBlockBack(blockResult);


                sprintf(buffer, "%s队%s防守拦回球：%s（质量值：%d）",
                        (currentDefendingTeam == 0) ? "A" : "B",
                        defenseResult.defender.name.c_str(),
                        defenseResult.description.c_str(),
                        defenseResult.qualityValue);
                emitUIEvent(buffer);


                if (defenseResult.quality == DEFENSE_FAULT) {
                    // 防守失误，对方得分
                    char buffer[256];
                    sprintf(buffer, "防守拦回球失误！%s队得分", (currentAttackingTeam == 0) ? "A" : "B");
                    emitUIEvent(buffer);

                    return currentAttackingTeam;
                }

                // 防守成功，准备下一轮进攻
                // 将防守结果转换为接一结果
                currentReceiveResult = convertDefenseToReceive(defenseResult);

                // 交换攻防角色：防守成功方变为进攻方
                std::swap(currentAttackingTeam, currentDefendingTeam);

                // 继续循环
                continue;
            }

            case BLOCK_BREAK: {
                // 拦网破坏，扣球强度增加
                char buffer[256];
                sprintf(buffer, "拦网破坏！扣球强度从%d增加到%d",
                        spikeResult.spikePower, blockResult.increasedSpikePower);
                emitUIEvent(buffer);

                // 更新扣球强度
                spikeResult.spikePower = blockResult.increasedSpikePower;

                // 继续进入防守环节
                break;
            }

            case LIMIT_PATH: {
                // 限制球路，扣球强度略微削减
                char buffer[256];
                sprintf(buffer, "限制球路！扣球强度从%d削减到%d",
                        spikeResult.spikePower, blockResult.reducedSpikePower);
                emitUIEvent(buffer);

                // 更新扣球强度
                spikeResult.spikePower = blockResult.reducedSpikePower;

                // 继续进入防守环节
                break;
            }

            case BLOCK_TOUCH: {
                // 撑起，扣球强度被削弱
                char buffer[256];
                sprintf(buffer, "扣球被撑起，强度从%d削弱到%d",
                        spikeResult.spikePower, blockResult.reducedSpikePower);
                emitUIEvent(buffer);

                // 更新扣球强度
                spikeResult.spikePower = blockResult.reducedSpikePower;

                // 继续进入防守环节
                break;
            }

            case NO_TOUCH: {
                // 无接触，扣球强度不变
                char buffer[256];
                sprintf(buffer, "无接触，扣球强度保持%d", spikeResult.spikePower);
                emitUIEvent(buffer);

                break;
            }
        }

            #if PAUSE_FOR_READ
                    system("pause");
            #endif

        // 6. 防守扣球（撑起或无接触的情况）
        Defender defender(game, currentDefendingTeam, currentAttackingTeam);
        DefenseResult defenseResult = defender.simulateDefenseAgainstSpike(spikeResult, blockResult);


        sprintf(buffer, "%s队%s防守：%s（质量值：%d）",
                (currentDefendingTeam == 0) ? "A" : "B",
                defenseResult.defender.name.c_str(),
                defenseResult.description.c_str(),
                defenseResult.qualityValue);
        emitUIEvent(buffer);


        if (defenseResult.quality == DEFENSE_FAULT) {
            // 防守失误，对方得分
            char buffer[256];
            sprintf(buffer, "防守失误！%s队得分", (currentAttackingTeam == 0) ? "A" : "B");
            emitUIEvent(buffer);

            if(currentAttackingTeam == 0) {
                game.scoredA[attackerID]++;
                // std::cout << "当前进攻得分者：" << teamA[attackerID].name << std::endl;
            } else {
                game.scoredB[attackerID]++;
                // std::cout << "当前进攻得分者：" << teamB[attackerID].name << std::endl;
            }

            return currentAttackingTeam;
        }

        // 防守成功，交换攻防角色
        std::swap(currentAttackingTeam, currentDefendingTeam);

        // 将防守结果转换为接一结果，用于下一轮进攻
        currentReceiveResult = convertDefenseToReceive(defenseResult);

        // 继续循环

            #if PAUSE_FOR_READ
                    system("pause");
            #endif
    }

    // 如果达到最大循环次数，随机决定得分方（防止无限循环）
    char buffer[256];
    sprintf(buffer, "攻防回合过多（超过%d回合），随机决定得分方", MAX_RALLY_COUNT);
    emitUIEvent(buffer);

    return (rand() % 2 == 0) ? currentAttackingTeam : currentDefendingTeam;
}

int processSimulation(GameState& game, Player& server, std::string serverTeam) {
    // 调用新的攻防循环函数
    return processRallyFromServe(game);
}

int playSet(int target, GameState& game) {
    game.scoreA = 0;
    game.scoreB = 0;

    char buffer1[256];
    sprintf(buffer1, "第%d局开始（目标%d分，领先2分获胜）", game.setNum, target);
    emitUIEvent(buffer1);
    char buffer2[256];
    sprintf(buffer2, "初始发球方：%s队", game.serveSide == 0 ? "A" : "B");
    emitUIEvent(buffer2);

    while(true) {
        // 检查获胜条件
        if((game.scoreA >= target || game.scoreB >= target) && abs(game.scoreA - game.scoreB) >= 2) {
            char buffer[256];
            sprintf(buffer, "第%d局结束！A队%d分，B队%d分", game.setNum, game.scoreA, game.scoreB);
            emitUIEvent(buffer);

            return game.scoreA > game.scoreB ? 0 : 1;
        }

        // 当前发球方
        std::string serverTeam = (game.serveSide == 0) ? "A队" : "B队";

        Player server;
        if(game.serveSide == 0) {
            server = teamA[game.rotateA[0]];  // A队1号位发球
        } else {
            server = teamB[game.rotateB[0]];  // B队1号位发球
        }

        char buffer[256];
        sprintf(buffer, "【当前比分：A:%d - B:%d】", game.scoreA, game.scoreB);
        emitUIEvent(buffer);

        printf("【当前阵容】\n");
        std::cout << std::setw(6) << teamA[game.rotateA[4]].name << " " << std::setw(6) << teamA[game.rotateA[3]].name << " | ";
        std::cout << std::setw(6) << teamB[game.rotateB[1]].name << " " << std::setw(6) << teamB[game.rotateB[0]].name << "\n";
        std::cout << std::setw(6) << teamA[game.rotateA[5]].name << " " << std::setw(6) << teamA[game.rotateA[2]].name << " | ";
        std::cout << std::setw(6) << teamB[game.rotateB[2]].name << " " << std::setw(6) << teamB[game.rotateB[5]].name << "\n";
        std::cout << std::setw(6) << teamA[game.rotateA[0]].name << " " << std::setw(6) << teamA[game.rotateA[1]].name << " | ";
        std::cout << std::setw(6) << teamB[game.rotateB[3]].name << " " << std::setw(6) << teamB[game.rotateB[4]].name << "\n";

        int scorer = -1;

        //模拟过程
        scorer = processSimulation(game, server, serverTeam);

        if(scorer == 0) {  // A队得分
            game.scoreA++;
            emitUIEvent("A队得分！");


            if(game.serveSide == 0) {
                // A队是发球方，得分后不轮转，发球人不变
            } else {
                // B队是发球方，A队获得发球权
                rotateTeam(game, 0);  // A队轮转
                game.serveSide = 0;       // 发球权交给A队

                if(server.position == "MB") {//B队副攻发球轮结束
                    game.liberoReplaceB = game.rotateB[0];
                    game.rotateB[0] = 6;
                }
            }
        } else {  // B队得分
            game.scoreB++;
            emitUIEvent("B队得分！");


            if(game.serveSide == 1) {
                // B队是发球方，得分后不轮转，发球人不变
            } else {
                // A队是发球方，B队获得发球权
                rotateTeam(game, 1);  // B队轮转
                game.serveSide = 1;       // 发球权交给B队

                if(server.position == "MB") {//A队副攻发球轮结束
                    game.liberoReplaceA = game.rotateA[0];
                    game.rotateA[0] = 6;
                }
            }
        }

#if PAUSE_EVERY_SCORE
        system("pause");
        #endif
    }
}

void newGame() {
    GameState game;
    // 随机决定初始发球方（0=A，1=B）
    game.serveSide = rand() % 2;
    char buffer[256];
    sprintf(buffer, "比赛开始！第一局发球方：%s", game.serveSide == 0 ? "A队" : "B队");
    emitUIEvent(buffer);

    inputPlayer();

    //初始化轮转位置
    for(int i = 0; i < 6; i++) {
        game.rotateA[i] = i;
        game.rotateB[i] = i;
    }

    // 初始化自由人替换
    if(teamA[game.rotateA[5]].position == "MB") {
        game.liberoReplaceA = game.rotateA[5];
        game.rotateA[5] = 6;
    }
    if(teamA[game.rotateA[4]].position == "MB") {
        game.liberoReplaceA = game.rotateA[4];
        game.rotateA[4] = 6;
    }
    if(teamA[game.rotateA[0]].position == "MB" && game.serveSide != 0) {
        game.liberoReplaceA = game.rotateA[0];
        game.rotateA[0] = 6;
    }

    if(teamB[game.rotateB[5]].position == "MB") {
        game.liberoReplaceB = game.rotateB[5];
        game.rotateB[5] = 6;
    }
    if(teamB[game.rotateB[4]].position == "MB") {
        game.liberoReplaceB = game.rotateB[4];
        game.rotateB[4] = 6;
    }
    if(teamB[game.rotateB[0]].position == "MB" && game.serveSide != 1) {
        game.liberoReplaceB = game.rotateB[0];
        game.rotateB[0] = 6;
    }

    if(PRE_SEED == 0) {
        int seed = time(0);
        srand(seed);
        std::ofstream ofs("seeds.txt", std::ios::app);
        ofs << seed << std::endl;
        ofs.close();
    } else {
        srand(PRE_SEED);
    }

    // 打满三局
    int winnerSetA = 0, winnerSetB = 0;
    // 第一局（25分）
    game.setNum = 1;
    int set1Winner = playSet(25, game);
    set1Winner == 0 ? winnerSetA++ : winnerSetB++;


    sprintf(buffer, "第二局发球方：%s", game.serveSide == 0 ? "A队" : "B队");
    emitUIEvent(buffer);
    emitUIEvent("请重新输入双方轮次");

    inputPlayer();
    //初始化轮转位置
    for(int i = 0; i < 6; i++) {
        game.rotateA[i] = i;
        game.rotateB[i] = i;
    }

    // 第二局（25分）
    game.setNum = 2;
    // 交换发球权
    game.serveSide = 1 - game.serveSide;
    int set2Winner = playSet(25, game);
    set2Winner == 0 ? winnerSetA++ : winnerSetB++;

    // 第三局（15分）
    game.setNum = 3;
    game.serveSide = rand() % 2;


    sprintf(buffer, "第三局发球方：%s", game.serveSide == 0 ? "A队" : "B队");
    emitUIEvent(buffer);
    emitUIEvent("请重新输入双方轮次");

    inputPlayer();
    //初始化轮转位置
    for(int i = 0; i < 6; i++) {
        game.rotateA[i] = i;
        game.rotateB[i] = i;
    }

    int set3Winner = playSet(15, game);
    set3Winner == 0 ? winnerSetA++ : winnerSetB++;

    // 全场结果
    emitUIEvent("全场比赛结束！");
    char buffer1[256];
    sprintf(buffer1, "A队胜%d局，B队胜%d局", winnerSetA, winnerSetB);
    emitUIEvent(buffer1);
    char buffer2[256];
    sprintf(buffer2, "最终胜者：%s队", winnerSetA > winnerSetB ? "A" : "B");
    emitUIEvent(buffer2);


    emitUIEvent("数据统计");

    emitUIEvent("A队：");
    for(int i = 0; i < 7; i++) {
        char buffer[256];
        sprintf(buffer, "%s|%s|进攻得分：%d|失误：%d",
                teamA[i].name.c_str(),
                teamA[i].position.c_str(),
                game.scoredA[i],
                game.faultA[i]);
        emitUIEvent(buffer);
    }
    emitUIEvent("B队：");
    for(int i = 0; i < 7; i++) {
        char buffer[256];
        sprintf(buffer, "%s|%s|进攻得分：%d|失误：%d",
                teamB[i].name.c_str(),
                teamB[i].position.c_str(),
                game.scoredB[i],
                game.faultB[i]);
        emitUIEvent(buffer);
    }

}