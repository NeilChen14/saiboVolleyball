package com.example.entity;

/**
 * @param power 威力，描述一个球的速度和力量
 * @param disruptiveness 破坏性，描述一个球的运动复杂程度，处理难度
 * @param distance 距离，描述一个球的运动距离
 */
public record Ball(double power, double disruptiveness, double distance, BallType type) {
}
