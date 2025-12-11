#include "setBall.h"
#include "config.h"
#include <algorithm>
#include <vector>
#include <cmath>
#include <iostream>


// 构造函数
Setter::Setter(const Player& setterPlayer, const GameState& gameState, int teamID)
    : setter(setterPlayer), gameState(gameState), teamID(teamID) {}

// 获取队伍球员数组
const Player* Setter::getTeamPlayers() {
    return (teamID == 0) ? teamA : teamB;
}

// 获取轮转数组
const int* Setter::getRotation() {
    return (teamID == 0) ? gameState.rotateA : gameState.rotateB;
}

// 判断二传是否在前排
bool Setter::isSetterInFrontRow() {
    const int* rotation = getRotation();
    const Player* team = getTeamPlayers();

    // 前排位置：4号位、3号位、2号位（数组索引3,2,1）
    for (int i : {3, 2, 1}) {
        if (team[rotation[i]].position == "S") {
            return true;
        }
    }
    return false;
}

// 计算传球调整系数
double Setter::calculatePassAdjustment(const ReceiveResult& receiveResult) {
    double adjustment = 1.0;

    // 基础调整：耐力影响
    double staminaEffect = sqrt(sqrt(setter.stamina / 100.0));
    double setFatigue = 1.0 - (gameState.setNum - 1) * 0.1;
    adjustment *= staminaEffect * setFatigue;

    // 心理素质影响
    double pressureEffect = setter.mental.pressureResist / 100.0;
    adjustment *= (0.85 + 0.3 * pressureEffect);

    // 专注度影响
    double concentrationEffect = setter.mental.concentration / 100.0;
    adjustment *= (0.85 + 0.3 * concentrationEffect);

    // 沟通配合影响
    double communicationEffect = setter.mental.commu_and_teamwork / 100.0;
    adjustment *= (0.9 + 0.2 * communicationEffect);

    // 调整属性影响：一传质量越差，调整属性影响越大
    double adjustEffect = setter.adjust / 100.0;
    double receiveQualityFactor = (100.0 - receiveResult.qualityValue) / 100.0; // 一传质量越差，值越大

    // 调整属性的影响权重随一传质量变化
    double adjustWeight = 0.3 + receiveQualityFactor * 0.4; // 基础30%，最大增加到70%
    adjustment *= (1.0 + adjustWeight * adjustEffect);

    double finalAdjustment = std::max(0.3, adjustment);

    #if DEBUG_SETBALL
    std::cout << "\n=== 传球调整系数调试信息 ===" << std::endl;
    std::cout << "二传球员: " << setter.name << " 传球属性: " << setter.pass << " 调整属性: " << setter.adjust << std::endl;
    std::cout << "1. 耐力影响: " << staminaEffect << " * 疲劳系数 " << setFatigue << " = " << staminaEffect * setFatigue << std::endl;
    std::cout << "2. 心理素质影响: " << pressureEffect << " => " << (0.85 + 0.3 * pressureEffect) << std::endl;
    std::cout << "3. 专注度影响: " << concentrationEffect << " => " << (0.85 + 0.3 * concentrationEffect) << std::endl;
    std::cout << "4. 沟通配合影响: " << communicationEffect << " => " << (0.9 + 0.2 * communicationEffect) << std::endl;
    std::cout << "5. 调整属性影响:" << std::endl;
    std::cout << "   一传质量值: " << receiveResult.qualityValue << " => 一传质量因子: " << receiveQualityFactor << std::endl;
    std::cout << "   调整属性值: " << setter.adjust << " => 调整效果: " << adjustEffect << std::endl;
    std::cout << "   调整权重: 基础0.3 + " << receiveQualityFactor << "*0.4 = " << adjustWeight << std::endl;
    std::cout << "   调整部分: 1 + "  << adjustWeight << "*" << adjustEffect << " = " << (1.0 + adjustWeight * adjustEffect) << std::endl;
    std::cout << "计算后调整系数: " << adjustment << std::endl;
    std::cout << "最终调整系数(限定范围): " << finalAdjustment << std::endl;
    std::cout << "============================" << std::endl;
    #endif

    return finalAdjustment;
}


// 计算二次进攻调整系数
double Setter::calculateDumpAdjustment(const ReceiveResult& receiveResult) {
    double adjustment = 1.0;

    // 耐力影响
    double staminaEffect = sqrt(sqrt(setter.stamina / 100.0));
    double setFatigue = 1.0 - (gameState.setNum - 1) * 0.1;
    adjustment *= staminaEffect * setFatigue;

    // 心理素质影响
    double pressureEffect = setter.mental.pressureResist / 100.0;
    adjustment *= (0.6 + 0.8 * pressureEffect); // 二次进攻更需要心理素质

    // 专注度影响
    double concentrationEffect = setter.mental.concentration / 100.0;
    adjustment *= (0.7 + 0.6 * concentrationEffect);

    // 沟通配合影响（二次进攻也需要观察对方拦网）
    double communicationEffect = setter.mental.commu_and_teamwork / 100.0;
    adjustment *= (0.9 + 0.2 * communicationEffect);

    // 调整属性影响：一传质量越差，二次进攻越难
    double adjustEffect = setter.adjust / 100.0;
    double receiveQualityFactor = (100.0 - receiveResult.qualityValue) / 100.0;
    double adjustWeight = 0.4 + receiveQualityFactor * 0.3; // 基础40%，最大增加到70%
    adjustment *= (1.0 - adjustWeight + adjustWeight * adjustEffect);

    double finalAdjustment = std::max(0.2, adjustment);
    
    #if DEBUG_SETBALL
    std::cout << "\n=== 二次进攻调整系数调试信息 ===" << std::endl;
    std::cout << "二传球员: " << setter.name << " 扣球属性: " << setter.spike << std::endl;
    std::cout << "1. 耐力影响: " << staminaEffect << " * 疲劳系数 " << setFatigue << " = " << staminaEffect * setFatigue << std::endl;
    std::cout << "2. 心理素质影响: " << pressureEffect << " => " << (0.6 + 0.4 * pressureEffect) << std::endl;
    std::cout << "3. 专注度影响: " << concentrationEffect << " => " << (0.7 + 0.3 * concentrationEffect) << std::endl;
    std::cout << "4. 沟通配合影响: " << communicationEffect << " => " << (0.8 + 0.2 * communicationEffect) << std::endl;
    std::cout << "5. 调整属性影响:" << std::endl;
    std::cout << "   一传质量因子: " << receiveQualityFactor << std::endl;
    std::cout << "   调整权重: 基础0.4 + " << receiveQualityFactor << "*0.3 = " << adjustWeight << std::endl;
    std::cout << "   调整部分: (1 - " << adjustWeight << ") + " << adjustWeight << "*" << adjustEffect << " = " << (1.0 - adjustWeight + adjustWeight * adjustEffect) << std::endl;
    std::cout << "最终调整系数(限定范围): " << finalAdjustment << std::endl;
    std::cout << "==================================" << std::endl;
    #endif
    
    return finalAdjustment;
}

// 评估攻手有效性
double Setter::calculateAttackerEffectiveness(const Player& attacker, int position) {
    // 1. 对位优势计算 (55%权重)
    double matchupAdvantage = 0.0;
    const Player* opponentTeam = (teamID == 0) ? teamB : teamA;
    const int* opponentRotation = (teamID == 0) ? gameState.rotateB : gameState.rotateA;

    // 确定主要拦网位置（根据攻手位置判断可能的拦网者）
    int blockPosition = -1;
    if (position >= 1 && position <= 3) { // 前排进攻
        // 对应位置的拦网手
        blockPosition = 4 - position;
    } else { // 后排进攻
        // 根据攻手类型选择对应的拦网者
        if ((attacker.position == "OH" || attacker.position == "主攻") && (position == 0 || position == 4 || position == 5)) {
            // 后排主攻进攻：选择拦网能力最强的副攻
            int strongestMiddleBlockerPos = -1;
            double maxMiddleBlockAbility = -1.0;
            for (int i = 1; i <= 3; i++) {
                if (opponentRotation[i] >= 0 && opponentRotation[i] < 6) {
                    const Player& potentialBlocker = opponentTeam[opponentRotation[i]];
                    // 寻找副攻位置的球员
                    if (potentialBlocker.position == "MB" || potentialBlocker.position == "副攻") {
                        if (potentialBlocker.block > maxMiddleBlockAbility) {
                            maxMiddleBlockAbility = potentialBlocker.block;
                            strongestMiddleBlockerPos = i;
                        }
                    }
                }
            }
            // 如果找不到副攻，则选择拦网能力最强的前排球员
            if (strongestMiddleBlockerPos != -1) {
                blockPosition = strongestMiddleBlockerPos;
            } else {
                // 后备方案：选择拦网能力最强的前排球员
                int strongestBlockerPos = -1;
                double maxBlockAbility = -1.0;
                for (int i = 1; i <= 3; i++) {
                    if (opponentRotation[i] >= 0 && opponentRotation[i] < 6) {
                        double blockAbility = opponentTeam[opponentRotation[i]].block;
                        if (blockAbility > maxBlockAbility) {
                            maxBlockAbility = blockAbility;
                            strongestBlockerPos = i;
                        }
                    }
                }
                blockPosition = strongestBlockerPos;
            }
        } else if ((attacker.position == "OP" || attacker.position == "接应") && (position == 0 || position == 4 || position == 5)) {
            // 后排接应进攻：选择拦网能力最强的主攻
            int strongestOHBlockerPos = -1;
            double maxOHBlockAbility = -1.0;
            for (int i = 1; i <= 3; i++) {
                if (opponentRotation[i] >= 0 && opponentRotation[i] < 6) {
                    const Player& potentialBlocker = opponentTeam[opponentRotation[i]];
                    // 寻找主攻位置的球员
                    if (potentialBlocker.position == "OH" || potentialBlocker.position == "主攻") {
                        if (potentialBlocker.block > maxOHBlockAbility) {
                            maxOHBlockAbility = potentialBlocker.block;
                            strongestOHBlockerPos = i;
                        }
                    }
                }
            }
            // 如果找不到主攻，则选择拦网能力最强的前排球员
            if (strongestOHBlockerPos != -1) {
                blockPosition = strongestOHBlockerPos;
            } else {
                // 后备方案：选择拦网能力最强的前排球员
                int strongestBlockerPos = -1;
                double maxBlockAbility = -1.0;
                for (int i = 1; i <= 3; i++) {
                    if (opponentRotation[i] >= 0 && opponentRotation[i] < 6) {
                        double blockAbility = opponentTeam[opponentRotation[i]].block;
                        if (blockAbility > maxBlockAbility) {
                            maxBlockAbility = blockAbility;
                            strongestBlockerPos = i;
                        }
                    }
                }
                blockPosition = strongestBlockerPos;
            }
        } else {
            // 其他后排位置：选择拦网能力最强的前排球员
            int strongestBlockerPos = -1;
            double maxBlockAbility = -1.0;
            for (int i = 1; i <= 3; i++) {
                if (opponentRotation[i] >= 0 && opponentRotation[i] < 6) {
                    double blockAbility = opponentTeam[opponentRotation[i]].block;
                    if (blockAbility > maxBlockAbility) {
                        maxBlockAbility = blockAbility;
                        strongestBlockerPos = i;
                    }
                }
            }
            blockPosition = strongestBlockerPos;
        }
    }

    // 计算对位优势
    if (blockPosition != -1 && opponentRotation[blockPosition] >= 0 && opponentRotation[blockPosition] < 6) {
        const Player& blocker = opponentTeam[opponentRotation[blockPosition]];
        // 攻手扣球能力与对手拦网能力的差值比例
        if (blocker.block > 0) {
            matchupAdvantage = (double)(attacker.spike - blocker.block) / 100.0;
            // 确保在合理范围内
            matchupAdvantage = std::max(-1.0, std::min(1.0, matchupAdvantage));
            // 转换为得分（55分满分）
            matchupAdvantage = (matchupAdvantage + 1.0) * 27.5;
        } else if(attacker.gender == 0 && blocker.gender == 1 && FEMALE_CAN_BE_BLOCK == 0) {
            matchupAdvantage = 55.0; // 对手男生不能拦女生，最大优势
        } else {
            matchupAdvantage = 55.0; // 对手无拦网能力，最大优势
        }
    } else {
        matchupAdvantage = 27.5; // 默认中等优势
    }

    // 2. 位置优势 (35%权重)
    double positionBonus = 0.0;
    if (position >= 1 && position <= 3) { // 前排位置
        // 2号位和3号位（副攻和接应位置）优势更大
        if (position == 2 || position == 3) {
            positionBonus = 35.0;
        } else { // 1号位（主攻位置）
            positionBonus = 30.0;
        }
    } else { // 后排位置
        // 后排位置基础优势较低，但不同后排位置有差异
        if (position == 5) { // 后排主攻
            positionBonus = 12.0;
        } else { // 其他后排位置
            positionBonus = 10.0;
        }
    }

    // 3. 体能因素 (10%权重) - 使用攻手当前体能
    double staminaBonus = (double)attacker.stamina * 0.1;

    // 计算总有效性分数
    double totalEffectiveness = matchupAdvantage + positionBonus + staminaBonus;

#if DEBUG_SETBALL
    std::cout << "攻手有效性评估: " << attacker.name << " (" << position+1 << "号位)" << std::endl;
    std::cout << "  对位优势: " << matchupAdvantage << " (55%)" << std::endl;
    std::cout << "  位置优势: " << positionBonus << " (35%)" << std::endl;
    std::cout << "  体能因素: " << staminaBonus << " (10%)" << std::endl;
    std::cout << "  总有效性: " << totalEffectiveness << std::endl;
#endif

    return totalEffectiveness;
}

PassTarget Setter::decidePassTarget(const ReceiveResult& receiveResult) {
    // 根据一传质量决定传球策略
    double randomValue = (rand() % 100) / 100.0;
    PassTarget target;

    // 将变量初始化移到switch语句之前
    std::vector<std::pair<PassTarget, double>> effectivenessList;
    const Player* team = getTeamPlayers();
    const int* rotation = getRotation();

    // 检查接应是否在后排（即二传在前排）
    bool oppositeInBackRow = isSetterInFrontRow();

    #if DEBUG_SETBALL
    std::cout << "\n=== 传球目标决策调试信息 ===" << std::endl;
    std::cout << "一传质量: ";
    switch(receiveResult.quality) {
        case RECEIVE_PERFECT: std::cout << "到位"; break;
        case RECEIVE_GOOD: std::cout << "半到位"; break;
        case RECEIVE_BAD: std::cout << "不到位"; break;
        case RECEIVE_FAULT: std::cout << "接飞"; break;
    }
    std::cout << " (质量值: " << receiveResult.qualityValue << ")" << std::endl;
    std::cout << "随机值: " << randomValue << std::endl;
    std::cout << "接应在后排: " << (oppositeInBackRow ? "是" : "否") << std::endl;
    #endif

    switch (receiveResult.quality)
    {
        case RECEIVE_PERFECT:{  // 到位
            // 可以组织快攻
            // 这里使用effectivenessList和calculateAttackerEffectiveness来选择最佳攻手
                // 遍历前排位置（1,2,3号位）找到副攻、接应和主攻
                Player frontBlocker, opposite, frontSpiker;
                bool foundFrontBlocker = false, foundOpposite = false, foundFrontSpiker = false;
                int oppositePosition = 0;

                for (int i = 1; i <= 3; i++) {
                    const Player& player = team[rotation[i]];
                    if ((player.position == "MB" || player.position == "副攻") && !foundFrontBlocker) {
                        frontBlocker = player;
                        foundFrontBlocker = true;
                    } else if ((player.position == "OP" || player.position == "接应") && !foundOpposite) {
                        opposite = player;
                        foundOpposite = true;
                        oppositePosition = 1;
                    } else if ((player.position == "OH" || player.position == "主攻") && !foundFrontSpiker) {
                        frontSpiker = player;
                        foundFrontSpiker = true;
                    }
                }
                // 遍历后排位置（0,4,5号位）找到后排主攻、接应
                Player backSpiker;
                bool foundBackSpiker = false;
                for (int i : {0, 4, 5}) {
                    const Player& player = team[rotation[i]];
                    if ((player.position == "OH" || player.position == "主攻") && !foundBackSpiker) {
                        backSpiker = player;
                        foundBackSpiker = true;
                    }else if ((player.position == "OP" || player.position == "接应") && !foundOpposite) {
                        opposite = player;
                        foundOpposite = true;
                    }
                }
            // 快攻给副攻，1.2倍加成
            double frontBlockerEffectiveness = calculateAttackerEffectiveness(frontBlocker, 2);
            effectivenessList.push_back(std::make_pair(FRONT_BLOCKER, frontBlockerEffectiveness * 1.2));

            // 接应
            double oppositeEffectiveness = calculateAttackerEffectiveness(opposite, oppositePosition);
            effectivenessList.push_back(std::make_pair(OPPOSITE, oppositeEffectiveness));

            // 前排主攻
            double frontSpikerEffectiveness = calculateAttackerEffectiveness(frontSpiker, 3);
            effectivenessList.push_back(std::make_pair(FRONT_SPIKER, frontSpikerEffectiveness));

            //后排主攻
            double backSpikerEffectiveness = calculateAttackerEffectiveness(backSpiker, 5);
            effectivenessList.push_back(std::make_pair(BACK_SPIKER, backSpikerEffectiveness));

            // 排序选择最高效的
            std::sort(effectivenessList.begin(), effectivenessList.end(),
                      [](const auto& a, const auto& b) { return a.second > b.second; });

            target = effectivenessList[0].first;
            break;
        }

        case RECEIVE_GOOD:  // 半到位
            if (randomValue < 0.8) {
                // 80%概率在前排和接应中选择
                // 如果接应在后排，降低传给接应的概率
                double oppositeProbability = oppositeInBackRow ? 0.10 : 0.35; // 从35%降至10%
                if (randomValue < oppositeProbability) {
                    target = OPPOSITE;
                } else { // 剩余概率给前排主攻
                    target = FRONT_SPIKER;
                }
            } else if (randomValue < 0.9) { // 10%概率给后排主攻
                target = BACK_SPIKER;
            } else { // 10%概率二次进攻
                target = isSetterInFrontRow() ? SETTER_DUMP : FRONT_SPIKER;
            }
            break;

        case RECEIVE_BAD:  // 不到位
            if (randomValue < 0.7) {
                // 70%概率给调整攻或接应
                // 如果接应在后排，降低传给接应的概率
                double oppositeProbability = oppositeInBackRow ? 0.1 : 0.3; // 从30%降至10%
                if (randomValue < oppositeProbability) {
                    target = OPPOSITE;
                } else { // 剩余概率给调整攻
                    target = ADJUST_ATTACK;
                }
            } else if (randomValue < 0.8) { // 10%概率给后排主攻
                target = BACK_SPIKER;
            } else { // 20%概率二次进攻
                target = isSetterInFrontRow() ? SETTER_DUMP : ADJUST_ATTACK;
            }
            break;

        default:  // 接飞不会到这里，但安全处理
            target = ADJUST_ATTACK;
    }

    #if DEBUG_SETBALL
    std::cout << "最终传球目标: ";
    switch(target) {
        case FRONT_SPIKER: std::cout << "前排主攻"; break;
        case FRONT_BLOCKER: std::cout << "前排副攻"; break;
        case BACK_SPIKER: std::cout << "后排主攻"; break;
        case OPPOSITE: std::cout << "接应"; break;
        case SETTER_DUMP: std::cout << "二传二次进攻"; break;
        case ADJUST_ATTACK: std::cout << "调整攻"; break;
    }
    std::cout << std::endl << "================================" << std::endl;
    #endif
    
    return target;
}

// 获取目标球员
Player Setter::getTargetPlayer(PassTarget target) {
    const Player* team = getTeamPlayers();
    const int* rotation = getRotation();
    Player result;

    // 根据目标类型找到对应球员
    switch (target) {
        case FRONT_SPIKER:  // 前排主攻
            // 寻找前排主攻（位置索引1,2,3中的主攻）
            for (int i = 1; i <= 3; i++) {
                if (team[rotation[i]].position == "OH" || team[rotation[i]].position == "主攻") {
                    result = team[rotation[i]];
                    #if DEBUG_SETBALL
                    std::cout << "找到前排主攻: " << result.name << " (" << i << "号位)" << std::endl;
                    #endif
                    return result;
                }
            }
            // 如果没找到，返回第一个前排球员
            result = team[rotation[1]];
            #if DEBUG_SETBALL
            std::cout << "未找到前排主攻，使用默认: " << result.name << " (1号位)" << std::endl;
            #endif
            return result;

        case FRONT_BLOCKER:  // 前排副攻
            // 寻找前排副攻
            for (int i = 1; i <= 3; i++) {
                if (team[rotation[i]].position == "MB" || team[rotation[i]].position == "副攻") {
                    result = team[rotation[i]];
                    #if DEBUG_SETBALL
                    std::cout << "找到前排副攻: " << result.name  << std::endl;
                    #endif
                    return result;
                }
            }
            // 如果没找到，返回第一个前排球员
            result = team[rotation[2]];
            #if DEBUG_SETBALL
            std::cout << "未找到前排副攻，使用默认: " << result.name << " (2号位)" << std::endl;
            #endif
            return result;

        case BACK_SPIKER:  // 后排主攻
            // 寻找后排主攻（位置索引0,4,5中的主攻）
            for (int i : {0, 4, 5}) {
                if (team[rotation[i]].position == "OH" || team[rotation[i]].position == "主攻") {
                    result = team[rotation[i]];
                    #if DEBUG_SETBALL
                    std::cout << "找到后排主攻: " << result.name << " (" << i << "号位)" << std::endl;
                    #endif
                    return result;
                }
            }
            // 如果没找到，返回第一个后排球员
            result = team[rotation[0]];
            #if DEBUG_SETBALL
            std::cout << "未找到后排主攻，使用默认: " << result.name << " (0号位)" << std::endl;
            #endif
            return result;

        case OPPOSITE:  // 接应
            // 寻找接应（无论前后排）
            for (int i = 0; i < 6; i++) {
                if (team[rotation[i]].position == "OP" || team[rotation[i]].position == "接应") {
                    result = team[rotation[i]];
                    #if DEBUG_SETBALL
                    std::cout << "找到接应: " << result.name << " (" << i << "号位)" << std::endl;
                    #endif
                    return result;
                }
            }
            // 如果没找到，返回第一个前排球员
            result = team[rotation[1]];
            #if DEBUG_SETBALL
            std::cout << "未找到接应，使用默认: " << result.name << " (1号位)" << std::endl;
            #endif
            return result;

        case SETTER_DUMP:  // 二传自己
            result = setter;
            #if DEBUG_SETBALL
            std::cout << "目标为二传自己: " << result.name << std::endl;
            #endif
            return result;

        case ADJUST_ATTACK:  // 调整攻，默认给主攻
        default:
            // 优先找前排主攻
            for (int i = 1; i <= 3; i++) {
               if (team[rotation[i]].position == "OH" || team[rotation[i]].position == "主攻") {
                   result = team[rotation[i]];
                   #if DEBUG_SETBALL
                   std::cout << "找到前排主攻(调整攻): " << result.name << " (" << i << "号位)" << std::endl;
                   #endif
                   return result;
               }
            }

            for (int i = 0; i < 6; i++) {
                if (team[rotation[i]].position == "OH" || team[rotation[i]].position == "主攻") {
                    result = team[rotation[i]];
                    #if DEBUG_SETBALL
                    std::cout << "找到主攻(调整攻): " << result.name << " (" << i << "号位)" << std::endl;
                    #endif
                    return result;
                }
            }
            for (int i = 0; i < 6; i++) {
                if (team[rotation[i]].position == "OP" || team[rotation[i]].position == "接应") {
                    result = team[rotation[i]];
                    #if DEBUG_SETBALL
                    std::cout << "找到接应(调整攻): " << result.name << " (" << i << "号位)" << std::endl;
                    #endif
                    return result;
                }
            }
            result = team[rotation[0]]; // 默认返回第一个球员
            #if DEBUG_SETBALL
            std::cout << "未找到合适球员，使用默认: " << result.name << " (0号位)" << std::endl;
            #endif
            return result;
    }
}

// 计算传球质量
PassQuality Setter::calculatePassQuality(const ReceiveResult& receiveResult, PassTarget target, int& qualityValue) {
    // 基础传球能力
    double basePassAbility = setter.pass;

    // 调整系数
    double adjustment = calculatePassAdjustment(receiveResult);

    // 一传质量影响
    double receiveInfluence = sqrt(receiveResult.qualityValue / 100.0);
    // 传球难度系数（不同目标难度不同）
    double difficultyFactor = 1.0;
    std::string difficultyStr = "标准";
    switch (target) {
        case FRONT_BLOCKER:  // 快攻最难
            difficultyFactor = SET_QUICK_ATTACK_DIFF_RATE;
            difficultyStr = "高(快攻)";
            break;
        case BACK_SPIKER:    // 后排攻较难
            difficultyFactor = SET_BACK_ATTACK_DIFF_RATE;
            difficultyStr = "较高(后排攻)";
            break;
        case OPPOSITE:       // 接应一般
            difficultyFactor = SET_OPPOSITE_DIFF_RATE;
            difficultyStr = "中等(接应)";
            break;
        case FRONT_SPIKER:   // 前排主攻较容易
            difficultyFactor = SET_FRONT_SPIKER_DIFF_RATE;
            difficultyStr = "较低(前排主攻)";
            break;
        case ADJUST_ATTACK:  // 调整攻最容易（只要高就行）
            difficultyFactor = SET_ADJUST_ATTACK_DIFF_RATE;
            difficultyStr = "低(调整攻)";
            break;
        default:
            difficultyFactor = 1.0;
            difficultyStr = "标准";
    }

    // 计算传球质量值
    double passValue = basePassAbility * adjustment * receiveInfluence / difficultyFactor;

    // 添加随机因素
    double randomFactor = (rand() % 20 - 10);
    passValue += randomFactor;
    passValue = std::max(0.0, passValue);

    qualityValue = static_cast<int>(passValue);

    // 根据质量值返回质量等级
    PassQuality quality;
    if (passValue >= 85) {
        quality = PERFECT_PASS;
    } else if (passValue >= 70) {
        quality = GOOD_PASS;
    } else if (passValue >= 50) {
        quality = DECENT_PASS;
    } else {
        quality = POOR_PASS;
    }
    
    #if DEBUG_SETBALL
    std::cout << "\n=== 传球质量计算调试信息 ===" << std::endl;
    std::cout << "基础传球能力: " << basePassAbility << std::endl;
    std::cout << "调整系数: " << adjustment << std::endl;
    std::cout << "一传影响: sqrt(" << receiveResult.qualityValue << "/100) = " << receiveInfluence << std::endl;
    std::cout << "传球难度系数: " << difficultyFactor << " (" << difficultyStr << ")" << std::endl;
    std::cout << "计算: " << basePassAbility << " * " << adjustment << " * " << receiveInfluence << " / " << difficultyFactor << " = " << basePassAbility * adjustment * receiveInfluence / difficultyFactor << std::endl;
    std::cout << "随机因素: " << randomFactor << std::endl;
    std::cout << "传球质量值: " << passValue << " (取整: " << qualityValue << ")" << std::endl;
    std::cout << "传球质量等级: ";
    switch(quality) {
        case PERFECT_PASS: std::cout << "完美传球"; break;
        case GOOD_PASS: std::cout << "好球"; break;
        case DECENT_PASS: std::cout << "一般传球"; break;
        case POOR_PASS: std::cout << "差球"; break;
    }
    std::cout << std::endl << "=================================" << std::endl;
    #endif

    return quality;
}

// 计算二次进攻质量
int Setter::calculateDumpQuality(const ReceiveResult& receiveResult, DumpType& dumpType) {
    // 计算球员的扣球和传球倾向分数
    double spikeScore = setter.spike * 0.7 + setter.mental.pressureResist * 0.3; // 扣球强且心理素质好的二传更倾向扣球
    double tipScore = setter.pass * 0.6 + setter.wisdom * 0.4; // 传球好且观察能力强的二传更倾向吊球

    // 添加球员个性因素：大多数二传倾向于吊球，扣球倾向降低30%
    spikeScore *= 0.7;

    // 根据分数决定二次进攻类型
    double randomValue = (rand() % 100) / 100.0;
    double totalScore = spikeScore + tipScore;
    double spikeProbability = spikeScore / totalScore;

    // 最终决定进攻类型
    if (randomValue < spikeProbability) {
        dumpType = SPIKE_DUMP;
    } else {
        dumpType = TIP_DUMP;
    }

    double baseAbility;
    std::string attackType;

    if (dumpType == SPIKE_DUMP) {
        // 二次扣球：基于扣球能力，保持1.5倍加成
        baseAbility = (double)setter.spike * 1.5;
        attackType = "二次扣球";
    } else {
        // 二次吊球：基于传球能力，1.2倍加成
        baseAbility = (double)setter.pass * 1.2;
        // 观察能力加成：gameReading提供额外50%潜力
        double observationBonus = 1.0 + (setter.wisdom / 100.0) * 0.5;
        baseAbility *= observationBonus;
        attackType = "二次吊球";
    }

    // 调整系数
    double adjustment = calculateDumpAdjustment(receiveResult);

    // 一传质量影响
    double receiveInfluence = sqrt(receiveResult.qualityValue / 100.0);

    // 计算二次进攻效果值
    double dumpValue = baseAbility * adjustment * receiveInfluence;

    // 添加随机因素
    double randomFactor = (rand() % 30 - 15);
    dumpValue += randomFactor;
    dumpValue = std::max(0.0, std::min(100.0, dumpValue));

    int result = static_cast<int>(dumpValue);

    #if DEBUG_SETBALL
    std::cout << "\n=== 二次进攻质量计算调试信息 ===" << std::endl;
    std::cout << "二传球员: " << setter.name << " 扣球属性: " << setter.spike << " 传球属性: " << setter.pass << std::endl;
    std::cout << "球商: " << setter.wisdom << std::endl;
    std::cout << "进攻类型: " << attackType << std::endl;
    std::cout << "扣球倾向分数: " << spikeScore << " 吊球倾向分数: " << tipScore << std::endl;
    std::cout << "扣球概率: " << spikeProbability * 100 << "%" << std::endl;
    std::cout << "基础能力值: " << baseAbility << std::endl;
    std::cout << "调整系数: " << adjustment << std::endl;
    std::cout << "一传影响: " << receiveResult.qualityValue << "/100 = " << receiveInfluence << std::endl;
    std::cout << "计算: " << baseAbility << " * " << adjustment << " * " << receiveInfluence << " = " << baseAbility * adjustment * receiveInfluence << std::endl;
    std::cout << "随机因素: " << randomFactor << std::endl;
    std::cout << "二次进攻效果值: " << dumpValue << " (四舍五入: " << result << ")" << std::endl;
    std::cout << "==================================" << std::endl;
    #endif
    
    return result;
}

// 模拟传球过程
PassResult Setter::simulateSet(const ReceiveResult& receiveResult) {
    PassResult result;

    #if DEBUG_SETBALL
    std::cout << "\n\n=== 传球模拟开始 ===" << std::endl;
    std::cout << "二传球员: " << setter.name << " 传球属性: " << setter.pass << std::endl;
    std::cout << "一传质量值: " << receiveResult.qualityValue << " (";
    switch(receiveResult.quality) {
        case RECEIVE_PERFECT: std::cout << "到位"; break;
        case RECEIVE_GOOD: std::cout << "半到位"; break;
        case RECEIVE_BAD: std::cout << "不到位"; break;
        case RECEIVE_FAULT: std::cout << "接飞"; break;
    }
    std::cout << ")" << std::endl;
    #endif

    // 决定传球目标
    result.target = decidePassTarget(receiveResult);

    // 获取目标球员
    result.targetPlayer = getTargetPlayer(result.target);

    // 判断是否为二次进攻
    result.isSetterDump = (result.target == SETTER_DUMP);

    if (result.isSetterDump) {
        // 二次进攻
        DumpType dumpType;
        result.dumpEffectiveness = calculateDumpQuality(receiveResult, dumpType);
        result.qualityValue = result.dumpEffectiveness;

        // 根据效果值确定质量等级和描述
        std::string dumpTypeStr = (dumpType == SPIKE_DUMP) ? "二次扣球" : "二次吊球";

        if (result.dumpEffectiveness >= 80) {
            result.quality = PERFECT_PASS;
            result.description = "精彩的" + dumpTypeStr + "！";
        } else if (result.dumpEffectiveness >= 60) {
            result.quality = GOOD_PASS;
            result.description = "不错的" + dumpTypeStr;
        } else if (result.dumpEffectiveness >= 40) {
            result.quality = DECENT_PASS;
            result.description = "一般的" + dumpTypeStr;
        } else {
            result.quality = POOR_PASS;
            result.description = "失误的" + dumpTypeStr;
        }
    } else {
        // 正常传球
        result.dumpEffectiveness = 0;
        result.quality = calculatePassQuality(receiveResult, result.target, result.qualityValue);

        // 设置描述
        switch (result.quality) {
            case PERFECT_PASS:
                result.description = "完美的传球！";
                break;
            case GOOD_PASS:
                result.description = "好球！";
                break;
            case DECENT_PASS:
                result.description = "还可以的传球";
                break;
            case POOR_PASS:
                result.description = "传球失误！";
                break;
        }
    }

    #if DEBUG_SETBALL
    std::cout << "\n=== 传球最终结果 ===" << std::endl;
    std::cout << "传球目标: ";
    switch(result.target) {
        case FRONT_SPIKER: std::cout << "前排主攻"; break;
        case FRONT_BLOCKER: std::cout << "前排副攻"; break;
        case BACK_SPIKER: std::cout << "后排主攻"; break;
        case OPPOSITE: std::cout << "接应"; break;
        case SETTER_DUMP: std::cout << "二传二次进攻"; break;
        case ADJUST_ATTACK: std::cout << "调整攻"; break;
    }
    std::cout << " (" << result.targetPlayer.name << ")" << std::endl;
    std::cout << "是否为二次进攻: " << (result.isSetterDump ? "是" : "否") << std::endl;
    std::cout << "传球质量等级: ";
    switch(result.quality) {
        case PERFECT_PASS: std::cout << "完美传球"; break;
        case GOOD_PASS: std::cout << "好球"; break;
        case DECENT_PASS: std::cout << "一般传球"; break;
        case POOR_PASS: std::cout << "差球"; break;
    }
    std::cout << " (质量值: " << result.qualityValue << ")" << std::endl;
    if (result.isSetterDump) {
        std::cout << "二次进攻效果值: " << result.dumpEffectiveness << std::endl;
    }
    std::cout << "描述: " << result.description << std::endl;
    std::cout << "=== 传球模拟结束 ===\n" << std::endl;
    #endif

    return result;
}
// [file content end]