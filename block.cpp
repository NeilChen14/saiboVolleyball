//
// Created by yaorz2 on 25-12-1.
//

#include "block.h"
#include "config.h"
#include <algorithm>
#include <cmath>
#include <iostream>

// 构造函数
Blocker::Blocker(const GameState& gameState, int blockingTeam, int attackingTeam)
    : gameState(gameState), blockingTeam(blockingTeam), attackingTeam(attackingTeam) {}

// 获取队伍球员数组
const Player* Blocker::getTeamPlayers(int teamID) {
    return (teamID == 0) ? teamA : teamB;
}

// 获取轮转数组
const int* Blocker::getRotation(int teamID) {
    return (teamID == 0) ? gameState.rotateA : gameState.rotateB;
}

// 判断球员是否在前排
bool Blocker::isFrontRowPlayer(const Player& player, int teamID) {
    const int* rotation = getRotation(teamID);
    const Player* team = getTeamPlayers(teamID);

    // 前排位置：4号位、3号位、2号位（数组索引3,2,1）
    for (int i : {3, 2, 1}) {
        if (team[rotation[i]].name == player.name) {
            return true;
        }
    }
    return false;
}

// 判断球员是否在后排
bool Blocker::isBackRowPlayer(const Player& player, int teamID) {
    const int* rotation = getRotation(teamID);
    const Player* team = getTeamPlayers(teamID);

    // 后排位置：5号位、6号位、1号位（数组索引4,5,0）
    for (int i : {4, 5, 0}) {
        if (team[rotation[i]].name == player.name) {
            return true;
        }
    }
    return false;
}

// 获取指定位置索引的球员
Player Blocker::getPlayerAtPosition(int teamID, int positionIndex) {
    const int* rotation = getRotation(teamID);
    const Player* team = getTeamPlayers(teamID);
    return team[rotation[positionIndex]];
}

// 获取前排主攻
Player Blocker::getFrontSpiker(int teamID) {
    const int* rotation = getRotation(teamID);
    const Player* team = getTeamPlayers(teamID);

    // 前排位置：2号位、3号位、4号位中寻找主攻
    for (int i : {1, 2, 3}) {
        Player player = team[rotation[i]];
        if (player.position == "OH" || player.position == "主攻") {
            return player;
        }
    }
    // 如果没有找到主攻，返回第一个前排球员
    return getPlayerAtPosition(teamID, 1);
}

// 获取前排副攻
Player Blocker::getFrontBlocker(int teamID) {
    const int* rotation = getRotation(teamID);
    const Player* team = getTeamPlayers(teamID);

    // 前排位置：2号位、3号位、4号位中寻找副攻
    for (int i : {1, 2, 3}) {
        Player player = team[rotation[i]];
        if (player.position == "MB" || player.position == "副攻") {
            return player;
        }
    }
    // 如果没有找到副攻，返回第二个前排球员
    return getPlayerAtPosition(teamID, 2);
}

// 获取接应
Player Blocker::getOpposite(int teamID) {
    const int* rotation = getRotation(teamID);
    const Player* team = getTeamPlayers(teamID);

    // 全场寻找接应
    for (int i = 0; i < 6; i++) {
        Player player = team[rotation[i]];
        if (player.position == "OP" || player.position == "接应") {
            return player;
        }
    }
    // 如果没有找到接应，返回第一个球员
    return getPlayerAtPosition(teamID, 0);
}

// 获取二传
Player Blocker::getSetter(int teamID) {
    const int* rotation = getRotation(teamID);
    const Player* team = getTeamPlayers(teamID);

    // 全场寻找二传
    for (int i = 0; i < 6; i++) {
        Player player = team[rotation[i]];
        if (player.position == "S" || player.position == "二传") {
            return player;
        }
    }
    // 如果没有找到二传，返回第一个球员
    return getPlayerAtPosition(teamID, 0);
}

// 根据进攻类型确定拦网人数
BlockType Blocker::determineBlockType(const SpikeResult& spikeResult) {
    Player attacker = spikeResult.attacker;
    bool isFrontRow = isFrontRowPlayer(attacker, attackingTeam);
    bool isBackRow = isBackRowPlayer(attacker, attackingTeam);

    // 检查是否是二次进攻（通过策略判断）
    bool isSetterDump = (attacker.position == "S" || attacker.position == "二传") &&
                        (spikeResult.strategy == QUICK_ATTACK || spikeResult.strategy == SETTER_SPIKE);

    // 获取拦网系数，用于判断进攻难度（值越低越难拦）
    double blockCoefficient = spikeResult.blockCoefficient;
    // 判断是否为快球或战术球
    bool isQuickAttack = (spikeResult.strategy == QUICK_ATTACK);
    // 判断是否为高质量传球或快传球（通过拦网系数判断，小于0.8表示较难拦的球）
    bool isHighQualityOrQuick = (blockCoefficient < 0.8) || isQuickAttack;

    BlockType blockType;

    #if DEBUG_BLOCK
    std::cout << "\n=== 拦网类型决策调试信息 ===" << std::endl;
    std::cout << "进攻球员: " << attacker.name << " 位置: " << attacker.position << std::endl;
    std::cout << "进攻位置: " << (isFrontRow ? "前排" : (isBackRow ? "后排" : "未知")) << std::endl;
    std::cout << "是否为二次进攻: " << (isSetterDump ? "是" : "否") << std::endl;
    std::cout << "是否为快球/战术球: " << (isQuickAttack ? "是" : "否") << std::endl;
    std::cout << "拦网系数: " << blockCoefficient << " (越低越难拦)" << std::endl;
    std::cout << "是否为高质量/快传球: " << (isHighQualityOrQuick ? "是" : "否") << std::endl;
    std::cout << "扣球策略: ";
    switch(spikeResult.strategy) {
        case STRONG_ATTACK: std::cout << "强攻"; break;
        case AVOID_BLOCK: std::cout << "避手"; break;
        case DROP_SHOT: std::cout << "吊球"; break;
        case QUICK_ATTACK: std::cout << "快球"; break;
        case ADJUST_SPIKE: std::cout << "调整攻"; break;
        case TRANSITION_ATTACK: std::cout << "过渡"; break;
        case SETTER_SPIKE: std::cout << "二次进攻"; break;
    }
    std::cout << std::endl;
    #endif

    // 根据进攻球员位置和角色确定拦网类型
    if (isSetterDump) {
        // 二次进攻：单人拦网
        blockType = SINGLE_BLOCK;
        #if DEBUG_BLOCK
        std::cout << "判定: 二次进攻 => 单人拦网" << std::endl;
        #endif
    } else if (attacker.position == "MB" || attacker.position == "副攻") {
        // 副攻进攻：通常单人拦网（敌方副攻）
        // 但在高质量/快传球或战术球时，可能拦网不到位（仍为单人但效率降低，通过blockCoefficient体现）
        blockType = SINGLE_BLOCK;
        #if DEBUG_BLOCK
        std::cout << "判定: 副攻进攻 => 单人拦网" << std::endl;
        if (isHighQualityOrQuick) {
            std::cout << "副攻快攻/高质量球，拦网难度增加" << std::endl;
        }
        #endif
    } else if (attacker.position == "OH" || attacker.position == "主攻" ||
               attacker.position == "OP" || attacker.position == "接应") {
        // 边攻（主攻和接应）：至少一人拦网
        // 正常球（不是快球且拦网系数较高）大概率两人拦网
        // 但受二传传球水平和敌方拦网水平影响，也可能只有一人拦网
        if (isHighQualityOrQuick) {
            // 高质量传球、快球或难拦的球，有更高概率单人拦网
            // 60%概率单人拦网，40%概率双人拦网
            if ((rand() % 100) < 60) {
                blockType = SINGLE_BLOCK;
                #if DEBUG_BLOCK
                std::cout << "判定: 边攻高质量/快传球 => 单人拦网" << std::endl;
                #endif
            } else {
                blockType = DOUBLE_BLOCK;
                #if DEBUG_BLOCK
                std::cout << "判定: 边攻高质量/快传球 => 双人拦网" << std::endl;
                #endif
            }
        } else {
            // 正常球，大概率两人拦网
            // 80%概率双人拦网，20%概率单人拦网
            if ((rand() % 100) < 80) {
                blockType = DOUBLE_BLOCK;
                #if DEBUG_BLOCK
                std::cout << "判定: 边攻正常球 => 双人拦网" << std::endl;
                #endif
            } else {
                blockType = SINGLE_BLOCK;
                #if DEBUG_BLOCK
                std::cout << "判定: 边攻正常球 => 单人拦网" << std::endl;
                #endif
            }
        }
    } else {
        // 默认：单人拦网或双人拦网，根据球的难度决定
        if (isHighQualityOrQuick) {
            blockType = SINGLE_BLOCK;
        } else {
            blockType = DOUBLE_BLOCK;
        }
        #if DEBUG_BLOCK
        std::cout << "判定: 默认 => " << (isHighQualityOrQuick ? "单人拦网" : "双人拦网") << std::endl;
        #endif
    }

    #if DEBUG_BLOCK
    std::cout << "最终拦网类型: ";
    switch(blockType) {
        case SINGLE_BLOCK: std::cout << "单人拦网"; break;
        case DOUBLE_BLOCK: std::cout << "双人拦网"; break;
        case TRIPLE_BLOCK: std::cout << "三人拦网"; break;
    }
    std::cout << std::endl << "==============================" << std::endl;
    #endif

    return blockType;
}

// 获取拦网球员
std::vector<Player> Blocker::getBlockers(BlockType blockType, const SpikeResult& spikeResult) {
    std::vector<Player> blockers;

    Player attacker = spikeResult.attacker;
    bool isFrontRow = isFrontRowPlayer(attacker, attackingTeam);

    // 检查是否是二次进攻
    bool isSetterDump = (spikeResult.strategy == QUICK_ATTACK &&
                        spikeResult.attacker.position == "S") ||
                       (spikeResult.attacker.position == "二传");

    #if DEBUG_BLOCK
    std::cout << "\n=== 拦网球员选择调试信息 ===" << std::endl;
    std::cout << "拦网类型: ";
    switch(blockType) {
        case SINGLE_BLOCK: std::cout << "单人拦网"; break;
        case DOUBLE_BLOCK: std::cout << "双人拦网"; break;
        case TRIPLE_BLOCK: std::cout << "三人拦网"; break;
    }
    std::cout << std::endl;
    std::cout << "是否为二次进攻: " << (isSetterDump ? "是" : "否") << std::endl;
    #endif

    // 根据拦网类型和进攻类型选择拦网球员
    switch (blockType) {
        case SINGLE_BLOCK:  // 单人拦网
            // 单人拦网情况下
            if (isSetterDump || (attacker.position == "OP" || attacker.position == "接应")) {
                // 二传扣球或接应扣球：选择对方前排主攻
                Player spiker = getFrontSpiker(blockingTeam);
                blockers.push_back(spiker);
                #if DEBUG_BLOCK
                std::cout << "选择拦网球员: " << spiker.name << " (前排主攻)" << std::endl;
                #endif
            } else if ((attacker.position == "OH" || attacker.position == "主攻")) {
                if (isFrontRow) {
                    // 前排主攻扣球：选择敌方二传和接应中在前排的球员（优先接应）
                    Player opposite = getOpposite(blockingTeam);
                    if (isFrontRowPlayer(opposite, blockingTeam)) {
                        // 接应由前排，则选择接应
                        blockers.push_back(opposite);
                        #if DEBUG_BLOCK
                        std::cout << "选择拦网球员: " << opposite.name << " (前排接应)" << std::endl;
                        #endif
                    } else {
                        // 接应不在前排，选择二传
                        Player setter = getSetter(blockingTeam);
                        if (isFrontRowPlayer(setter, blockingTeam)) {
                            blockers.push_back(setter);
                            #if DEBUG_BLOCK
                            std::cout << "选择拦网球员: " << setter.name << " (前排二传)" << std::endl;
                            #endif
                        } else {
                            // 如果二传也不在前排，选择前排副攻作为备选
                            Player blocker = getFrontBlocker(blockingTeam);
                            blockers.push_back(blocker);
                            #if DEBUG_BLOCK
                            std::cout << "选择拦网球员: " << blocker.name << " (前排副攻 - 二传和接应都不在前排)" << std::endl;
                            #endif
                        }
                    }
                } else {
                    // 后排主攻扣球：选择敌方副攻
                    Player blocker = getFrontBlocker(blockingTeam);
                    blockers.push_back(blocker);
                    #if DEBUG_BLOCK
                    std::cout << "选择拦网球员: " << blocker.name << " (前排副攻)" << std::endl;
                    #endif
                }
            } else {
                // 其他情况：选择前排副攻
                Player blocker = getFrontBlocker(blockingTeam);
                blockers.push_back(blocker);
                #if DEBUG_BLOCK
                std::cout << "选择拦网球员: " << blocker.name << " (前排副攻)" << std::endl;
                #endif
            }
            break;

        case DOUBLE_BLOCK:  // 双人拦网
            // 单人拦网的基础上添加副攻
            if (isSetterDump || (attacker.position == "OP" || attacker.position == "接应")) {
                // 二传扣球或接应扣球：前排主攻和副攻双人拦网
                Player spiker = getFrontSpiker(blockingTeam);
                Player blocker = getFrontBlocker(blockingTeam);
                blockers.push_back(spiker);
                blockers.push_back(blocker);
                #if DEBUG_BLOCK
                std::cout << "选择拦网球员: " << spiker.name << " (前排主攻)" << std::endl;
                std::cout << "选择拦网球员: " << blocker.name << " (前排副攻)" << std::endl;
                #endif
            } else if ((attacker.position == "OH" || attacker.position == "主攻")) {
                if (isFrontRow) {
                    // 前排主攻扣球：单人拦网选择的球员 + 敌方副攻
                    // 先获取单人拦网的球员
                    std::vector<Player> singleBlockers = getBlockers(SINGLE_BLOCK, spikeResult);
                    if (!singleBlockers.empty()) {
                        blockers.push_back(singleBlockers[0]);
                    }
                    // 添加敌方副攻
                    Player blocker = getFrontBlocker(blockingTeam);
                    blockers.push_back(blocker);
                    #if DEBUG_BLOCK
                    std::cout << "选择拦网球员: " << (singleBlockers.empty() ? "无人" : singleBlockers[0].name) << " (单人拦网选择)" << std::endl;
                    std::cout << "选择拦网球员: " << blocker.name << " (前排副攻)" << std::endl;
                    #endif
                } else {
                    // 后排主攻扣球：敌方副攻和主攻双人拦网
                    Player blocker = getFrontBlocker(blockingTeam);
                    Player spiker = getFrontSpiker(blockingTeam);
                    blockers.push_back(blocker);
                    blockers.push_back(spiker);
                    #if DEBUG_BLOCK
                    std::cout << "选择拦网球员: " << blocker.name << " (前排副攻)" << std::endl;
                    std::cout << "选择拦网球员: " << spiker.name << " (前排主攻)" << std::endl;
                    #endif
                }
            } else {
                // 对于其他位置的双人拦网情况，选择前排主攻和副攻
                Player spiker = getFrontSpiker(blockingTeam);
                Player blocker = getFrontBlocker(blockingTeam);
                blockers.push_back(spiker);
                blockers.push_back(blocker);
                #if DEBUG_BLOCK
                std::cout << "选择拦网球员: " << spiker.name << " (前排主攻)" << std::endl;
                std::cout << "选择拦网球员: " << blocker.name << " (前排副攻)" << std::endl;
                #endif
            }
            break;

        case TRIPLE_BLOCK:  // 三人拦网
            // 三人拦网：前排所有球员
            for (int i = 1; i <= 3; i++) {
                Player player = getPlayerAtPosition(blockingTeam, i);
                blockers.push_back(player);
                #if DEBUG_BLOCK
                std::cout << "选择拦网球员: " << player.name << " (前排位置" << i << ")" << std::endl;
                #endif
            }
            break;
    }

    #if DEBUG_BLOCK
    std::cout << "总共拦网球员数: " << blockers.size() << std::endl;
    std::cout << "===============================" << std::endl;
    #endif

    return blockers;
}

// 计算单个拦网球员的拦网强度
double Blocker::calculateSingleBlockPower(const Player& blocker, const SpikeResult& spikeResult) {
    double blockPower = 0.0;

    // 基础拦网属性
    double baseBlockAbility = blocker.block;

    // 团队协作属性影响较大（权重40%）
    double teamworkEffect = blocker.mental.commu_and_teamwork / 100.0;

    // 心理素质影响
    double pressureEffect = blocker.mental.pressureResist / 100.0;

    // 专注度影响
    double concentrationEffect = blocker.mental.concentration / 100.0;

    // 自信心影响
    double confidenceEffect = blocker.mental.confidence / 100.0;

    // 耐力影响
    double staminaEffect = blocker.stamina / 100.0;
    double setFatigue = 1.0 - (gameState.setNum - 1) * 0.1;

    // 计算综合调整系数
    double adjustment =
        (baseBlockAbility / 100.0) * 0.4 +           // 拦网技能40%
        teamworkEffect * 0.4 +                      // 团队协作40%
        pressureEffect * 0.1 +                      // 抗压能力10%
        concentrationEffect * 0.05 +                // 专注度5%
        confidenceEffect * 0.05;                    // 自信心5%

    // 应用耐力影响
    adjustment *= staminaEffect * setFatigue;

    // 拦网策略对特定扣球策略的适应性
    double strategyAdaptation = 1.0;
    std::string strategyEffectStr = "无";
    switch (spikeResult.strategy) {
        case QUICK_ATTACK:      // 快球：需要快速反应
            strategyAdaptation *= (0.7 + 0.3 * concentrationEffect);
            strategyEffectStr = "快球(需要快速反应)";
            break;
        case DROP_SHOT:         // 吊球：需要判断力
            strategyAdaptation *= (0.6 + 0.4 * concentrationEffect);
            strategyEffectStr = "吊球(需要判断力)";
            break;
        case AVOID_BLOCK:       // 避手：需要预判
            strategyAdaptation *= (0.5 + 0.5 * pressureEffect);
            strategyEffectStr = "避手(需要预判)";
            break;
        default:
            strategyAdaptation = 1.0;
            strategyEffectStr = "标准";
    }

    // 最终拦网强度
    blockPower = baseBlockAbility * adjustment * strategyAdaptation;

    double finalPower = std::max(0.0, std::min(100.0, blockPower));

    #if DEBUG_BLOCK
    std::cout << "\n=== 单个拦网球员强度调试信息 ===" << std::endl;
    std::cout << "拦网球员: " << blocker.name << " 拦网属性: " << blocker.block << std::endl;
    std::cout << "1. 基础拦网能力: " << baseBlockAbility << std::endl;
    std::cout << "2. 团队协作: " << teamworkEffect << " (权重40%)" << std::endl;
    std::cout << "3. 心理素质: " << pressureEffect << " (权重10%)" << std::endl;
    std::cout << "4. 专注度: " << concentrationEffect << " (权重5%)" << std::endl;
    std::cout << "5. 自信心: " << confidenceEffect << " (权重5%)" << std::endl;
    std::cout << "6. 耐力影响: " << staminaEffect << " * 疲劳系数 " << setFatigue << " = " << staminaEffect * setFatigue << std::endl;
    std::cout << "综合调整系数: (" << baseBlockAbility/100.0 << "*0.4 + " << teamworkEffect << "*0.4 + " << pressureEffect << "*0.1 + " << concentrationEffect << "*0.05 + " << confidenceEffect << "*0.05) * " << staminaEffect * setFatigue << " = " << adjustment << std::endl;
    std::cout << "策略适应性(" << strategyEffectStr << "): " << strategyAdaptation << std::endl;
    std::cout << "计算后拦网强度: " << baseBlockAbility << " * " << adjustment << " * " << strategyAdaptation << " = " << blockPower << std::endl;
    std::cout << "最终拦网强度(限定范围): " << finalPower << std::endl;
    std::cout << "=================================" << std::endl;
    #endif

    return finalPower;
}

// 计算组合拦网强度
int Blocker::calculateCombinedBlockPower(const std::vector<Player>& blockers, const SpikeResult& spikeResult) {
    if (blockers.empty()) {
        #if DEBUG_BLOCK
        std::cout << "无拦网球员，组合拦网强度为0" << std::endl;
        #endif
        return 0;
    }

    #if DEBUG_BLOCK
    std::cout << "\n=== 组合拦网强度计算调试信息 ===" << std::endl;
    std::cout << "拦网球员数量: " << blockers.size() << std::endl;
    #endif

    // 计算平均拦网强度
    double totalPower = 0.0;
    for (const auto& blocker : blockers) {
        double singlePower = calculateSingleBlockPower(blocker, spikeResult);
        totalPower += singlePower;
        #if DEBUG_BLOCK
        std::cout << "球员 " << blocker.name << " 单拦强度: " << singlePower << std::endl;
        #endif
    }
    double averagePower = totalPower / blockers.size();

    // 团队协作加成：拦网人数越多，团队协作影响越大
    double teamworkBonus = 0.0;
    for (const auto& blocker : blockers) {
        teamworkBonus += blocker.mental.commu_and_teamwork / 100.0;
    }
    double teamworkFactor = 1.0 + (teamworkBonus / blockers.size()) * 0.3;

    // 人数加成：多人拦网有协同效应
    double numberBonus = 0.7;
    switch (blockers.size()) {
        case 2:  // 双人拦网
            numberBonus = DOUBLE_BLOCK_RATE;
            break;
        case 3:  // 三人拦网
            numberBonus = TRIPLE_BLOCK_RATE;
            break;
        default: // 单人拦网
            numberBonus = SINGLE_BLOCK_RATE;
    }

    // 计算组合拦网强度
    double combinedPower = averagePower * teamworkFactor * numberBonus;

    // 添加随机因素
    double randomFactor = (rand() % 20 - 10);
    combinedPower += randomFactor;

    int finalPower = static_cast<int>(std::max(0.0, std::min(100.0, combinedPower)));

    #if DEBUG_BLOCK
    std::cout << "平均拦网强度: " << totalPower << " / " << blockers.size() << " = " << averagePower << std::endl;
    std::cout << "团队协作加成: 平均团队协作 " << teamworkBonus/blockers.size() << " => 系数 1 + " << teamworkBonus/blockers.size() << "*0.3 = " << teamworkFactor << std::endl;
    std::cout << "人数加成: " << blockers.size() << "人拦网 => 系数 " << numberBonus << std::endl;
    std::cout << "组合拦网强度: " << averagePower << " * " << teamworkFactor << " * " << numberBonus << " = " << combinedPower - randomFactor << std::endl;
    std::cout << "随机因素: " << randomFactor << std::endl;
    std::cout << "最终组合拦网强度: " << combinedPower << " (取整： " << finalPower << ")" << std::endl;
    std::cout << "====================================" << std::endl;
    #endif

    return finalPower;
}

// 计算拦网效果
double Blocker::calculateBlockEffect(int blockPower, int spikePower, double blockCoefficient) {
    if (spikePower == 0) {
        #if DEBUG_BLOCK
        std::cout << "扣球强度为0，拦网效果为0" << std::endl;
        #endif
        return 0.0; // 扣球失误，拦网效果为0
    }

    // 基础拦网效果 = 拦网强度 / 扣球强度
    double baseEffect = static_cast<double>(blockPower) / spikePower;

    // 应用拦网系数：系数越低，拦网效果越好
    double coefficientEffect = (1.5 - blockCoefficient); // blockCoefficient在0.3-1.5之间
    baseEffect *= coefficientEffect;

    // 添加随机因素
    double randomFactor = ((rand() % 20) - 10) / 100.0;
    baseEffect += randomFactor;

    // 限制效果值范围：0.0-1.0
    double finalEffect = std::max(0.0, std::min(1.0, baseEffect));

    #if DEBUG_BLOCK
    std::cout << "\n=== 拦网效果计算调试信息 ===" << std::endl;
    std::cout << "拦网强度: " << blockPower << std::endl;
    std::cout << "扣球强度: " << spikePower << std::endl;
    std::cout << "拦网系数: " << blockCoefficient << std::endl;
    std::cout << "基础拦网效果: " << blockPower << " / " << spikePower << " = " << static_cast<double>(blockPower)/spikePower << std::endl;
    std::cout << "拦网系数影响: 2.0 - " << blockCoefficient << " = " << coefficientEffect << std::endl;
    std::cout << "计算后效果: " << static_cast<double>(blockPower)/spikePower << " * " << coefficientEffect << " = " << baseEffect - randomFactor << std::endl;
    std::cout << "随机因素: " << randomFactor << std::endl;
    std::cout << "最终拦网效果: " << finalEffect << " (范围: 0.0-1.0)" << std::endl;
    std::cout << "=============================" << std::endl;
    #endif

    return finalEffect;
}

// 确定拦网结果
BlockResult Blocker::determineBlockResult(double blockEffect) {
    // 根据拦网效果决定结果
    double randomValue = (rand() % 100) / 100.0;
    BlockResult result;

    #if DEBUG_BLOCK
    std::cout << "\n=== 拦网结果判定调试信息 ===" << std::endl;
    std::cout << "拦网效果: " << blockEffect << std::endl;
    std::cout << "随机值: " << randomValue << std::endl;
    #endif

    // 调整判定阈值，使结果分布更合理
    if (blockEffect < BLOCK_BREAK_THRESHOLD) {
        // 效果很差，破坏
        result = BLOCK_BREAK;
        #if DEBUG_BLOCK
                std::cout << "拦网效果 < " << BLOCK_BREAK_THRESHOLD << " => 破坏" << std::endl;
        #endif
    } else if (blockEffect < BLOCK_NO_TOUCH_THRESHOLD) {
        // 效果差，无接触
        result = NO_TOUCH;
        #if DEBUG_BLOCK
                std::cout << BLOCK_BREAK_THRESHOLD << " ≤ 拦网效果 < " << BLOCK_NO_TOUCH_THRESHOLD << " => 无接触" << std::endl;
        #endif
    } else if (blockEffect < BLOCK_LIMIT_PATH_THRESHOLD) {
        // 效果一般，限制球路
        result = LIMIT_PATH;
        #if DEBUG_BLOCK
                std::cout << BLOCK_NO_TOUCH_THRESHOLD << " ≤ 拦网效果 < " << BLOCK_LIMIT_PATH_THRESHOLD << " => 限制球路" << std::endl;
        #endif
    } else if (blockEffect < BLOCK_TOUCH_THRESHOLD) {
        // 效果中等，撑起
        result = BLOCK_TOUCH;
        #if DEBUG_BLOCK
                std::cout << BLOCK_LIMIT_PATH_THRESHOLD << " ≤ 拦网效果 < " << BLOCK_TOUCH_THRESHOLD << " => 撑起" << std::endl;
        #endif
    } else {
        // 效果好，拦回
        result = BLOCK_BACK;
        #if DEBUG_BLOCK
                std::cout << "拦网效果 ≥ " << BLOCK_TOUCH_THRESHOLD << " => 拦回" << std::endl;
        #endif
    }

    #if DEBUG_BLOCK
        std::cout << "拦网结果: ";
        switch(result) {
        case BLOCK_BREAK: std::cout << "破坏"; break;
        case NO_TOUCH: std::cout << "无接触"; break;
        case LIMIT_PATH: std::cout << "限制球路"; break;
        case BLOCK_TOUCH: std::cout << "撑起"; break;
        case BLOCK_BACK: std::cout << "拦回"; break;
        }
        std::cout << std::endl << "==============================" << std::endl;
    #endif

    return result;
}

// 计算拦网破坏时增加的扣球强度
int Blocker::calculateIncreasedSpikePower(int spikePower, double blockEffect) {
    // 增加比例 = (1 - 拦网效果) * 0.3（最多增加30%）
    double increaseRatio = (1.0 - blockEffect) * SPIKE_INCREASE_RATIO;

    // 计算增加后的扣球强度
    double increasedPower = spikePower * (1.0 + increaseRatio);

    // 添加随机因素
    double randomFactor = (rand() % 10 - 5);
    increasedPower += randomFactor;

    int finalPower = static_cast<int>(std::max(0.0, increasedPower));

#if DEBUG_BLOCK
    std::cout << "\n=== 增加扣球强度计算调试信息 ===" << std::endl;
    std::cout << "原始扣球强度: " << spikePower << std::endl;
    std::cout << "拦网效果: " << blockEffect << std::endl;
    std::cout << "增加比例: (1 - " << blockEffect << ") * 0.3 = " << increaseRatio << std::endl;
    std::cout << "增加后强度: " << spikePower << " * (1 + " << increaseRatio << ") = " << spikePower * (1.0 + increaseRatio) << std::endl;
    std::cout << "随机因素: " << randomFactor << std::endl;
    std::cout << "最终增加后强度: " << increasedPower << " (四舍五入: " << finalPower << ")" << std::endl;
    std::cout << "==================================" << std::endl;
#endif

    return finalPower;
}

// 计算撑起时削减的扣球强度
int Blocker::calculateReducedSpikePower(int spikePower, double blockEffect) {
    // 削减比例 = 拦网效果 * 0.7（最多削减70%）
    double reductionRatio = blockEffect * SPIKE_REDUCTION_RATIO;

    // 计算削减后的扣球强度
    double reducedPower = spikePower * (1.0 - reductionRatio);

    // 添加随机因素
    double randomFactor = (rand() % 10 - 5);
    reducedPower += randomFactor;

    int finalPower = static_cast<int>(std::max(0.0,  reducedPower));

    #if DEBUG_BLOCK
    std::cout << "\n=== 削减扣球强度计算调试信息 ===" << std::endl;
    std::cout << "原始扣球强度: " << spikePower << std::endl;
    std::cout << "拦网效果: " << blockEffect << std::endl;
    std::cout << "削减比例: " << blockEffect << " * 0.7 = " << reductionRatio << std::endl;
    std::cout << "削减后强度: " << spikePower << " * (1 - " << reductionRatio << ") = " << spikePower * (1.0 - reductionRatio) << std::endl;
    std::cout << "随机因素: " << randomFactor << std::endl;
    std::cout << "最终削减后强度: " << reducedPower << " (四舍五入: " << finalPower << ")" << std::endl;
    std::cout << "==================================" << std::endl;
    #endif

    return finalPower;
}

// 计算拦回强度
int Blocker::calculateBlockBackPower(int blockPower, int spikePower) {
    // 拦回强度 = (拦网强度 * 0.6 + 扣球强度 * 0.4) * 0.8
    // 乘以0.8是因为拦回球通常速度较慢
    double blockBackPower = (blockPower * 0.6 + spikePower * 0.4) * 0.8;

    // 添加随机因素
    double randomFactor = (rand() % 20 - 10);
    blockBackPower += randomFactor;

    int finalPower = static_cast<int>(std::max(0.0, std::min(100.0, blockBackPower)));

    #if DEBUG_BLOCK
    std::cout << "\n=== 拦回强度计算调试信息 ===" << std::endl;
    std::cout << "拦网强度: " << blockPower << std::endl;
    std::cout << "扣球强度: " << spikePower << std::endl;
    std::cout << "计算: (" << blockPower << "*0.6 + " << spikePower << "*0.4) * 0.8 = " << (blockPower * 0.6 + spikePower * 0.4) * 0.8 << std::endl;
    std::cout << "随机因素: " << randomFactor << std::endl;
    std::cout << "最终拦回强度: " << blockBackPower << " (四舍五入: " << finalPower << ")" << std::endl;
    std::cout << "==============================" << std::endl;
    #endif

    return finalPower;
}

// 模拟拦网过程
BlockResultInfo Blocker::simulateBlock(const SpikeResult& spikeResult) {
    BlockResultInfo result;

    // 检查扣球是否失误
    if (spikeResult.isError) {
        result.description = "扣球失误，无拦网";
        result.result = NO_TOUCH;
        return result;
    }

    // 确定拦网类型
    BlockType blockType = determineBlockType(spikeResult);

    // 获取拦网球员
    result.blockers = getBlockers(blockType, spikeResult);

    // 确保边攻扣球时至少有一名拦网球员
    bool isWingAttacker = (spikeResult.attacker.position == "OH" || spikeResult.attacker.position == "主攻" ||
                          spikeResult.attacker.position == "OP" || spikeResult.attacker.position == "接应");

    if (isWingAttacker && result.blockers.empty()) {
        // 边攻扣球但没有拦网球员时，至少选择一名前排球员
        Player frontSpiker = getFrontSpiker(blockingTeam);
        result.blockers.push_back(frontSpiker);
        #if DEBUG_BLOCK
        std::cout << "边攻扣球，强制添加拦网球员: " << frontSpiker.name << std::endl;
        #endif
    }

    // 计算组合拦网强度
    result.blockPower = calculateCombinedBlockPower(result.blockers, spikeResult);

    // 计算拦网效果
    double blockEffect = calculateBlockEffect(result.blockPower, spikeResult.spikePower, spikeResult.blockCoefficient);

    // 确定拦网结果
    result.result = determineBlockResult(blockEffect);

    // 根据结果设置描述
    switch (result.result) {
    case BLOCK_BREAK: {
            // 计算增加后的扣球强度
            result.increasedSpikePower = calculateIncreasedSpikePower(spikeResult.spikePower, blockEffect);
            result.description = "拦网破坏！扣球威力增加";
            break;
    }
    case NO_TOUCH: {
            result.description = "无接触！扣球通过拦网";
            break;
    }
    case LIMIT_PATH: {
            // 计算略微削减后的扣球强度
            result.reducedSpikePower = calculateReducedSpikePower(spikeResult.spikePower, blockEffect * 0.3);
            result.description = "限制球路！扣球路线被限制";
            break;
    }
    case BLOCK_TOUCH: {
            // 计算削减后的扣球强度
            result.reducedSpikePower = calculateReducedSpikePower(spikeResult.spikePower, blockEffect);
            result.description = "拦网撑起！";
            break;
    }
    case BLOCK_BACK: {
            // 计算拦回强度
            result.blockBackPower = calculateBlockBackPower(result.blockPower, spikeResult.spikePower);
            result.description = "精彩拦回！";
            break;
    }
    }

    // 添加拦网人数信息
    std::string blockerNames = "";
    for (size_t i = 0; i < result.blockers.size(); i++) {
        if (i > 0) blockerNames += "、";
        blockerNames += result.blockers[i].name;
    }
    // 无论是否有拦网接触，都显示拦网球员信息
    if (!blockerNames.empty()) {
        std::string blockTypeStr = std::to_string(result.blockers.size()) + "人拦网";
        result.description += "（" + blockTypeStr + "：" + blockerNames + "）";
    } else if (isWingAttacker) {
        // 对于边攻，即使没有拦网球员也提示
        result.description += "（边攻扣球，应有拦网）";
    }


    #if DEBUG_BLOCK
    std::cout << "\n=== 拦网模拟最终结果 ===" << std::endl;
    std::cout << "拦网描述: " << result.description << std::endl;
    std::cout << "拦网结果: ";
    switch(result.result) {
    case BLOCK_BREAK: std::cout << "破坏"; break;
    case NO_TOUCH: std::cout << "无接触"; break;
    case LIMIT_PATH: std::cout << "限制球路"; break;
    case BLOCK_TOUCH: std::cout << "撑起"; break;
    case BLOCK_BACK: std::cout << "拦回"; break;
    }
    std::cout << std::endl;
    std::cout << "拦网强度: " << result.blockPower << std::endl;
    if (result.result == BLOCK_BREAK) {
        std::cout << "拦网破坏后扣球强度: " << result.increasedSpikePower << std::endl;
    } else if (result.result == LIMIT_PATH || result.result == BLOCK_TOUCH) {
        std::cout << "削减后扣球强度: " << result.reducedSpikePower << std::endl;
    } else if (result.result == BLOCK_BACK) {
        std::cout << "拦回强度: " << result.blockBackPower << std::endl;
    }
    std::cout << "============================" << std::endl;
    #endif

    return result;
}
