//
// Created by yaorz2 on 25-12-2.
//

#ifndef CONFIG_H
#define CONFIG_H

// ============   特殊设置   ============

#define PAUSE_EVERY_SCORE 0             //在每一球结束时停止以阅读

#define USE_PRESET_INPUT 1                // 使用预设的球员输入

#define PRE_SEED 0                        // 设置种子（0为不设置）

// ============ 全局调试开关 ============
// 设为1启用调试信息，设为0禁用

#define DEBUG_ALL 0 // 主开关，设为1启用所有调试信息

// 各模块调试开关
#define DEBUG_BLOCK (DEBUG_ALL || 0)       // 拦网模块
#define DEBUG_SERVE (DEBUG_ALL || 0)       // 发球模块
#define DEBUG_RECEIVE (DEBUG_ALL || 0)     // 接一模块
#define DEBUG_SETBALL (DEBUG_ALL || 0)     // 二传模块
#define DEBUG_SPIKE (DEBUG_ALL || 0)       // 扣球模块
#define DEBUG_DEFENSE (DEBUG_ALL || 0)     // 防守模块
#define DEBUG_GAME (DEBUG_ALL || 0)        // 比赛流程

#define DEBUG_MENTAL_CALC (DEBUG_ALL || 0)   // 心理参数计算

// 其他配置

#define PAUSE_FOR_READ (DEBUG_ALL || 0)    // 是否在每个回合后暂停

// ============   特殊规则   ============

//女生能否被男生拦网(0不可/1可以)
#define FEMALE_CAN_BE_BLOCK 0

// ============ 游戏平衡参数 ============
// 这些参数可以在运行时调整来平衡游戏
// 由于内容过多，且部分参数计算较复杂，故仅开放了一部分调整系数

// 二次进攻失误率 (0.0-1.0)
#define DUMP_ERROR_RATE 0.25

// 最大攻防回合数（防止无限循环）
#define MAX_RALLY_COUNT 500

// ------------ 发球部分 ------------

// 冲发球决策阈值（越低越容易冲发球
#define AGGRESSIVE_SERVE_THRESHOLD 0.5

//稳定发球基础失误率
#define STABLE_SERVE_BASIC_FAULT_RATE 0.15
//冲发球基础失误率
#define AGGRESSIVE_SERVE_BASIC_FAULT_RATE 0.3

//稳定发球基础倍率
#define STABLE_SERVE_POWER_RATE 0.7
//冲发球基础倍率
#define AGGRESSIVE_SERVE_POWER_RATE 1.2

// ------------ 接一部分 ------------

// ------------ 二传部分 ------------

//传球难度系数
//分别为：快攻、后排进攻、接应、主攻、调整攻
#define SET_QUICK_ATTACK_DIFF_RATE 1.4
#define SET_BACK_ATTACK_DIFF_RATE 1.4
#define SET_OPPOSITE_DIFF_RATE 1.2
#define SET_FRONT_SPIKER_DIFF_RATE 1.0
#define SET_ADJUST_ATTACK_DIFF_RATE 0.8

// ------------ 扣球部分 ------------
/*各策略属性系数：
 *POWER：力量属性，越高代表扣球强度越高
 *BLOCK：被拦属性：越高代表越容易被拦网
 *FAULT：失误属性：越高代表失误率越高
 *PASS：受二传影响属性：越高代表受二传影响越大
 *ADJUST：调整属性：越高代表调整属性影响越大
*/
//STRONG_ATTACK 强攻
#define STRONG_ATTACK_POWER 1.9
#define STRONG_ATTACK_BLOCK 1.0
#define STRONG_ATTACK_FAULT 0.2
#define STRONG_ATTACK_PASS 0.8
#define STRONG_ATTACK_ADJUST 0.3
//AVOID_BLOCK 避手
#define AVOID_BLOCK_POWER 1.8
#define AVOID_BLOCK_BLOCK 0.5
#define AVOID_BLOCK_FAULT 0.3
#define AVOID_BLOCK_PASS 0.6
#define AVOID_BLOCK_ADJUST 0.4
//DROP_SHOT 吊球
#define DROP_SHOT_POWER 1.0
#define DROP_SHOT_BLOCK 0.1
#define DROP_SHOT_FAULT 0.15
#define DROP_SHOT_PASS 0.3
#define DROP_SHOT_ADJUST 0.5
//QUICK_ATTACK 快球
#define QUICK_ATTACK_POWER 1.8
#define QUICK_ATTACK_BLOCK 0.8
#define QUICK_ATTACK_FAULT 0.2
#define QUICK_ATTACK_PASS 0.9
#define QUICK_ATTACK_ADJUST 0.5
//ADJUST_SPIKE 调整攻
#define ADJUST_SPIKE_POWER 1.6
#define ADJUST_SPIKE_BLOCK 1.0
#define ADJUST_SPIKE_FAULT 0.15
#define ADJUST_SPIKE_PASS 0.5
#define ADJUST_SPIKE_ADJUST 0.6
//TRANSITION_ATTACK 过渡
#define TRANSITION_ATTACK_POWER 1.3
#define TRANSITION_ATTACK_BLOCK 0.4
#define TRANSITION_ATTACK_FAULT 0.1
#define TRANSITION_ATTACK_PASS 0.2
#define TRANSITION_ATTACK_ADJUST 0.8
//SETTER_SPIKE 二次
#define SETTER_SPIKE_POWER 1.8
#define SETTER_SPIKE_BLOCK 0.8
#define SETTER_SPIKE_FAULT 0.25
#define SETTER_SPIKE_PASS 0.6
#define SETTER_SPIKE_ADJUST 0.6

//后排进攻补正：强度-25%，被拦网系数-30%
#define BACK_ATTACK_POWER_ADJUST 0.75
#define BACK_ATTACK_BLOCK_ADJUST 0.7

// ------------ 拦网部分 ------------

//多人拦网人数加成
#define SINGLE_BLOCK_RATE 0.7
#define DOUBLE_BLOCK_RATE 1.0
#define TRIPLE_BLOCK_RATE 1.4

// 拦网效果阈值（5级）
#define BLOCK_BREAK_THRESHOLD 0.2       // 破坏与无接触阈值
#define BLOCK_NO_TOUCH_THRESHOLD 0.4    // 无接触与限制球路阈值
#define BLOCK_LIMIT_PATH_THRESHOLD 0.6  // 限制球路与撑起阈值
#define BLOCK_TOUCH_THRESHOLD 0.8       // 撑起与拦回阈值

//拦网破坏时，扣球强度加成（0.0-1.0）
#define SPIKE_INCREASE_RATIO 1.0

// 拦网撑起时，扣球削弱比例 (0.0-1.0)
#define SPIKE_REDUCTION_RATIO 0.7

// ------------ 防守部分 ------------


// ============ 能力值映射函数 ============
// 分段线性映射函数：f(x)=If(x≤50,0.4x,50<x≤90,1.5x-55,x>90,2x-100)
inline int mapAbilityValue(int ability) {
    if (ability <= 50) {
        return static_cast<int>(0.4 * ability);
    } else if (ability <= 90) {
        return static_cast<int>(1.5 * ability - 55);
    } else {
        return static_cast<int>(2 * ability - 100);
    }
}

#endif //CONFIG_H