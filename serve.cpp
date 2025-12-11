#include "serve.h"
#include "config.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

// 添加调试信息标志

Serve::Serve(const Player& server, const GameState& game)
    : server(server), game(game), adjustment(0.0) {
}

ServeType Serve::decideServeStrategy() {
    // 基础策略：根据球员属性和比赛情况决定
    
    // 1. 发球属性高且自信心高的球员更倾向于冲发球
    double serveSkillFactor = (server.serve / 100.0 * 0.75 + server.mental.confidence / 100.0 * 0.25);
    
    // 2. 心理素质好的球员在压力下更倾向于稳定发球
    double mentalFactor = server.mental.pressureResist / 100.0;
    
    // 3. 专注度高的球员更可能选择冲发球
    double concentrationFactor = server.mental.concentration / 100.0;
    
    // 4. 比赛局势影响
    int scoreDiff = game.scoreA - game.scoreB;
    if (game.serveSide == 1) scoreDiff = -scoreDiff; // 对B队来说要取反
    
    double situationFactor;
    if (abs(scoreDiff) <= 2) {
        // 比分接近时，根据球员特点决定
        situationFactor = 0.5;
    } else if (scoreDiff > 0) {
        // 领先时更倾向于稳定发球
        situationFactor = 0.3;
    } else {
        // 落后时更倾向于冲发球搏杀
        situationFactor = 0.7;
    }
    
    // 5. 耐力影响：耐力低的球员后期更倾向于稳定发球
    double staminaFactor = server.stamina / 100.0;
    double fatigueEffect = 1.0 - (game.setNum - 1) * 0.2; // 每局耐力影响增加
    
    // 综合决策
    double aggressiveTendency = 
        serveSkillFactor * 0.4 +           // 发球技能权重40%
        concentrationFactor * 0.2 +        // 专注度权重20%
        situationFactor * 0.2 +            // 局势权重20%
        (1 - mentalFactor) * 0.1 +         // 心理素质反向权重10%
        staminaFactor * fatigueEffect * 0.1; // 耐力权重10%
    
    // 添加随机因素
    double randomFactor = (rand() % 20 - 10) / 100.0;
    aggressiveTendency += randomFactor;
    
    ServeType result = (aggressiveTendency > AGGRESSIVE_SERVE_THRESHOLD) ? AGGRESSIVE_SERVE : STABLE_SERVE;
    
    #if DEBUG_SERVE
    std::cout << "\n=== 发球策略决策调试信息 ===" << std::endl;
    std::cout << "球员: " << server.name << " 发球属性: " << server.serve << " 自信: " << server.mental.confidence << std::endl;
    std::cout << "1. 发球技能因子: " << serveSkillFactor << std::endl;
    std::cout << "2. 心理素质因子: " << mentalFactor << std::endl;
    std::cout << "3. 专注度因子: " << concentrationFactor << std::endl;
    std::cout << "4. 比分差: " << scoreDiff << " 局势因子: " << situationFactor << std::endl;
    std::cout << "5. 耐力因子: " << staminaFactor << " 疲劳影响: " << fatigueEffect << std::endl;
    std::cout << "6. 随机因子: " << randomFactor << std::endl;
    std::cout << "综合攻击倾向: " << aggressiveTendency << " (阈值: 0.5)" << std::endl;
    std::cout << "最终决策: " << (result == AGGRESSIVE_SERVE ? "冲发球" : "稳定发球") << std::endl;
    std::cout << "===========================" << std::endl;
    #endif
    
    return result;
}

double Serve::calculateServeAdjustment() {
    double adjustment = 1.0;
    
    // 耐力影响：比赛越久，耐力越低，失误率增加
    double staminaEffect = sqrt(sqrt(server.stamina / 100.0));
    double setFatigue = 1.0 - (game.setNum - 1) * 0.1; // 每局增加10%的疲劳
    adjustment *= staminaEffect * setFatigue;
    
    // 心理素质影响
    double mentalEffect = server.mental.pressureResist / 100.0;
    adjustment *= (0.85 + 0.3 * mentalEffect); // 心理素质占30%权重
    
    // 专注度影响
    double concentrationEffect = server.mental.concentration / 100.0;
    adjustment *= (0.9 + 0.2 * concentrationEffect); // 专注度占20%权重

    double randomEffect = (rand() % 20 - 10) / 100.0;
    adjustment *= (1.0 + randomEffect);
    
    #if DEBUG_SERVE
    std::cout << "\n=== 发球调整系数调试信息 ===" << std::endl;
    std::cout << "1. 耐力影响: " << staminaEffect << " * 疲劳系数 " << setFatigue << " = " << staminaEffect * setFatigue << std::endl;
    std::cout << "2. 心理素质影响: " << mentalEffect << " => " << (0.85 + 0.3 * mentalEffect) << std::endl;
    std::cout << "3. 专注度影响: " << concentrationEffect << " => " << (0.9 + 0.2 * concentrationEffect) << std::endl;
    std::cout << "3. 专注度影响: " << concentrationEffect << " => " << (0.9 + 0.2 * concentrationEffect) << std::endl;
    std::cout << "4. 随机影响：" << randomEffect << " => " << (1.0 + randomEffect) << std::endl;
    std::cout << "最终调整系数: " << adjustment << std::endl;
    std::cout << "===========================" << std::endl;
    #endif
    
    return std::max(0.3, adjustment); // 确保调整系数不低于0.3
}

int Serve::calculateServePower() {
    int basePower = server.serve;
    int finalPower;
    
    switch(serveType) {
        case STABLE_SERVE:
            // 稳定发球：中等强度，受调整影响较小
            finalPower = static_cast<int>(basePower * 0.7 * (0.8 + 0.2 * adjustment));
            #if DEBUG_SERVE
            std::cout << "\n稳定发球: 基础强度" << basePower << " * 0.7 * (" << 0.8 << " + 0.2*" << adjustment << ") = " << finalPower << std::endl;
            #endif
            return finalPower;
        case AGGRESSIVE_SERVE:
            // 冲发球：高强度，受调整影响较大
            finalPower = static_cast<int>(basePower * 1.2 * adjustment);
            #if DEBUG_SERVE
            std::cout << "\n冲发球: 基础强度" << basePower << " * 1.2 * " << adjustment << " = " << finalPower << std::endl;
            #endif
            return finalPower;
        default:
            return basePower;
    }
}

double Serve::calculateServeFaultRate() {
    double baseFaultRate;
    
    switch(serveType) {
        case STABLE_SERVE:
            baseFaultRate = STABLE_SERVE_BASIC_FAULT_RATE; // 15%基础失误率
            break;
        case AGGRESSIVE_SERVE:
            baseFaultRate = AGGRESSIVE_SERVE_BASIC_FAULT_RATE; // 30%基础失误率
            break;
        default:
            baseFaultRate = 0.2;
    }
    
    // 发球属性降低失误率
    double serveSkillEffect = server.serve / 100.0;
    double faultReduction = serveSkillEffect * 0.5; // 发球技能最多减少50%失误率
    
    // 应用调整系数
    double adjustedFaultRate = baseFaultRate * (2.0 - adjustment) * faultReduction;
    
    double finalFaultRate = std::max(0.05, std::min(0.8, adjustedFaultRate)); // 失误率限制在5%-80%之间
    
    #if DEBUG_SERVE
    std::cout << "\n=== 发球失误率调试信息 ===" << std::endl;
    std::cout << "基础失误率: " << baseFaultRate << " (" << (serveType == STABLE_SERVE ? "稳定" : "冲发") << ")" << std::endl;
    std::cout << "发球技能效果: " << serveSkillEffect << " => 失误率减少: " << faultReduction << "%" << std::endl;
    std::cout << "调整系数影响: (2.0 - " << adjustment << ") = " << (2.0 - adjustment) << std::endl;
    std::cout << "计算后失误率: " << adjustedFaultRate << std::endl;
    std::cout << "最终失误率(限定范围): " << finalFaultRate << std::endl;
    std::cout << "===========================" << std::endl;
    #endif
    
    return finalFaultRate;
}

ServeResult Serve::simulate() {
    ServeResult result;

#if DEBUG_SERVE
    std::cout << "\n\n=== 发球模拟开始 ===" << std::endl;
    std::cout << "发球球员: " << server.name << " 发球属性: " << server.serve << std::endl;
    std::cout << "当前局数: " << game.setNum << " 比分 A:" << game.scoreA << " B:" << game.scoreB << std::endl;
#endif

    // 决定发球策略
    serveType = decideServeStrategy();
    result.type = serveType;

    // 计算调整系数
    adjustment = calculateServeAdjustment();

    // 计算发球强度和失误率
    int servePower = calculateServePower();
    double faultRate = calculateServeFaultRate();

    // 判断发球是否成功
    double randomValue = (rand() % 100) / 100.0;
    result.success = (randomValue > faultRate);

#if DEBUG_SERVE
    std::cout << "\n=== 发球结果 ===" << std::endl;
    std::cout << "随机值: " << randomValue << " 失误率阈值: " << faultRate << std::endl;
    std::cout << "发球是否成功: " << (result.success ? "成功" : "失败") << std::endl;
#endif

    if (!result.success) {
        result.effectiveness = 0;
#if DEBUG_SERVE
        std::cout << "发球失误，效果值为0" << std::endl;
        std::cout << "=== 发球模拟结束 ===\n" << std::endl;
#endif
        return result;
    }

    // 发球成功，计算发球效果
    result.effectiveness = servePower;

#if DEBUG_SERVE
    std::cout << "发球效果值: " << result.effectiveness << std::endl;
    std::cout << "=== 发球模拟结束 ===\n" << std::endl;
#endif

    return result;
}
