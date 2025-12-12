package com.example.entity;

import com.example.group.Role;

import java.util.Random;

import static java.lang.Math.exp;
import static java.lang.Math.pow;

public class Player {
    private final String name;

    //技术熟练(0-100)
    private final int serve;  //发
    private final int pass;   //垫
    private final int set;    //传
    private final int spike;  //扣
    private final int block;  //拦

    //基本能力(0-100)
    private final int strength;          //力量
    private final int height;            //高度
    private final int flexibility;       //灵活
    private final int reliable;          //稳定
    private final int stamina_resist;    //体力抵抗
    private final int mentality_resist;  //心态抵抗

    //当前状态：影响力量、高度、灵活、稳定
    private double stamina;    //体力
    private double mentality;  //心态

    Random random = new Random();
    NormalDistribution normalDistribution = new NormalDistribution();

    public Player (String name,
                   int serve, int pass, int set, int spike, int block,
                   int strength, int reliable, int flexibility, int height,
                   int stamina, int mentality) {
        this.name = name;

        this.serve = serve;
        this.pass = pass;
        this.set = set;
        this.spike = spike;
        this.block = block;

        this.strength = strength;
        this.height = height;
        this.flexibility = flexibility;
        this.reliable = reliable;
        this.stamina_resist = stamina;
        this.mentality_resist = mentality;

        this.stamina = 100.0;
        this.mentality = 100.0;
    }

    public String getName() {
        return name;
    }

    //发球
    public Ball serve() {
        //跳发
        boolean isJump = getServe() * 0.2
                + getAbility(0.4, 0.4, 0, 0)
                > fluctuation(80);

        //飘球
        boolean isFloat = getServe() * 0.3
                + getAbility(0.1, 0, 0.4, 0.2)
                > fluctuation(80);

        BallType type = isJump
                ? (isFloat ? BallType.JUMP_FLOAT_SERVE : BallType.JUMP_SERVE)
                : (isFloat ? BallType.FLOAT_SERVE : BallType.SERVE);

        //质量参数
        double quality = getServe();

        //威力
        double power = quality * 0.2 + switch (type) {
            case BallType.SERVE ->
                    getAbility(0.5, 0.1, 0.1, 0.1);
            case BallType.JUMP_SERVE ->
                    getAbility(0.6, 0.3, 0.05, 0.05);
            case BallType.FLOAT_SERVE ->
                    getAbility(0.3, 0.1, 0.2, 0.2);
            case BallType.JUMP_FLOAT_SERVE ->
                    getAbility(0.3, 0.4, 0.15, 0.15);
            default -> 0;
        };
        power = power < 0 ? 0 : power;

        //破坏性
        double disruptiveness = quality * 0.3 + switch (type) {
            case BallType.SERVE, JUMP_SERVE ->
                    getAbility(0.1, 0.1, 0.4, 0.1);
            case BallType.FLOAT_SERVE ->
                    getAbility(0.05, 0.05, 0.6, 0.2);
            case JUMP_FLOAT_SERVE ->
                    getAbility(0.05, 0.25, 0.5, 0.1);
            default -> 0;
        };
        disruptiveness = disruptiveness < 0 ? 0 : disruptiveness;

        //距离
        double distance = deviation(4.5 + (isJump ? 1.5 : 0) - (isFloat ? 1.5 : 0),
                2.5, power, disruptiveness);

        //体力减少
        staminaChange(1 - power / 10000);

        return new Ball(power, disruptiveness, distance, type);
    }

    //除探头球外所有第一次触球
    public Ball pass(Ball ball) {
        //成功接球
        double abilityFactor = getPass() * 0.35 +
                getAbility(0, 0, 0.35, 0.45);
        double challengeFactor = ball.power() * 0.6 + ball.disruptiveness() * 0.4;
        boolean isSuccess = (100 - 20 * exp((challengeFactor - abilityFactor) / 10)
                > random.nextDouble() * 100);
        if (challengeFactor - abilityFactor > 20) isSuccess = false;
        if (!isSuccess) return new Ball(0, 0, ball.distance(), BallType.FAILED_PASS);

        //质量参数
        double quality = getPass();

        //威力
        double power = ball.power() * (1 -
                (100 - ball.power() * 0.2 - ball.disruptiveness() * 0.1)
                        * (quality * 0.3 + getAbility(0.4, 0.05, 0.1, 0.15))
                        / 12500);

        //破坏性
        double disruptiveness = ball.disruptiveness() * (1 -
                (100 - ball.power() * 0.1 - ball.disruptiveness() * 0.2)
                        * (quality * 0.35 + getAbility(0.05, 0.1, 0.35, 0.15))
                        / 12500);

        //距离
        double distance = deviation(3,
                2.5, power, disruptiveness);

        BallType type = distance < 0 ? BallType.OVER_PASS :
                distance < 6 ? BallType.PASS : BallType.BAD_PASS;
        if (distance < 0) distance = -distance;

        //体力减少
        staminaChange(1 - (ball.power() - power) / 10000);

        return new Ball(power, disruptiveness, distance, type);
    }

    //所有第二次触球
    public Ball set(Ball ball, int position) {
        if (ball.type() == BallType.PASS) {
            //二次球
            boolean isAttack = !(ball.distance() > 1.5)
                    && 0.6 * (1 - ball.distance() / 1.5) * (100 + (
                    getAbility(0.15, 0.25, 0.25, 0.15)
                            + getSet() * 0.2 - ball.power() * 0.4 - ball.disruptiveness() * 0.6)) / 100
                    > random.nextDouble() * 100;

            //二次球
            if (isAttack && position > 1 && position < 5) {
                //质量参数
                double quality = getSet();

                //威力
                double power = quality * 0.3
                        + getAbility(0.4, 0.3, 0.2, 0.2)
                        - ball.power() * 0.7 - ball.disruptiveness() * 0.7;

                //破坏性
                double disruptiveness = quality * 0.3
                        + getAbility(0.2, 0.2, 0.4, 0.4)
                        - ball.power() * 0.2 - ball.disruptiveness() * 0.2;

                //距离
                double distance = deviation(1, 0.6, power, disruptiveness);
                if (distance < 0) return new Ball(0, 0, distance, BallType.UNDER_NET);

                //体力减少
                staminaChange(1 - power / 10000);

                return new Ball(power, disruptiveness, distance, BallType.ONE_PASS_ATTACK);
            } else { //二传
                //质量参数
                double quality = getSet();

                //威力
                double power = ball.power() * (1 -
                        (100 - ball.power() * 0.2 - ball.disruptiveness() * 0.1)
                                * (quality * 0.15 + getAbility(0.25, 0.05, 0.35, 0.20))
                                / 12500);

                //破坏性
                double disruptiveness = ball.disruptiveness() * (1 -
                        (100 - ball.power() * 0.1 - ball.disruptiveness() * 0.2)
                                * (quality * 0.2 + getAbility(0.1, 0.05, 0.4, 0.25))
                                / 12500);

                //距离
                double distance = deviation(3,
                        2.5, power, disruptiveness);

                BallType type = distance < 0 ? BallType.OVER_SET :
                        distance < 6 ? BallType.SET : BallType.BAD_SET;
                if (distance < 0) distance = -distance;

                //体力减少
                staminaChange(1 - (ball.power() - power) / 10000);

                return new Ball(power, disruptiveness, distance, type);
            }
        } else {
            return pass(ball);
        }
    }

    //所有第三次触球及探头球
    public Ball spike(Ball ball, int position, Role role) {
        if ((ball.type() == BallType.PASS || ball.type() == BallType.SET) && role != Role.L) {
            double quality = getSpike();
            if (role == Role.MB && position > 1 && position < 5) { //快攻
                //威力
                double power = quality * 0.25
                        + getAbility(0.35, 0.1, 0.5, 0.2)
                        - ball.power() * 0.8 - ball.disruptiveness() * 0.2;

                //破坏性
                double disruptiveness = quality * 0.3
                        + getAbility(0.25, 0.1, 0.45, 0.1)
                        - ball.power() * 0.2 - ball.disruptiveness() * 0.8;

                //距离
                double distance = deviation(3, 2.5, power, disruptiveness);

                BallType type = distance < 0 ? BallType.UNDER_NET :
                        distance > 9 ? BallType.OUT_BALL : BallType.SPIKE;

                //体力减少
                staminaChange(1 - power / 10000);

                return new Ball(power, disruptiveness, distance, type);
            } else { //扣杀
                //威力
                double power = quality * 0.2
                        + getAbility(0.45, 0.35, 0.1, 0.1)
                        - ball.power() * 0.8 - ball.disruptiveness() * 0.2;

                //破坏性
                double disruptiveness = quality * 0.5
                        + getAbility(0.2, 0.1, 0.45, 0.1)
                        - ball.power() * 0.2 - ball.disruptiveness() * 0.8;

                //距离
                double distance = deviation(4.5, 3, power, disruptiveness);

                BallType type = distance < 0 ? BallType.UNDER_NET :
                        distance > 9 ? BallType.OUT_BALL : BallType.SPIKE;

                //体力减少
                staminaChange(1 - power / 10000);

                return new Ball(power, disruptiveness, distance, type);
            }
        } else { //处理球
            //质量参数
            double quality = getPass();

            //威力
            double power = quality * 0.1
                    + getAbility(0.25, 0, 0, 0.15)
                    - ball.power();
            if (power < 0) power = -power;

            //破坏性
            double disruptiveness = quality * 0.15
                    + getAbility(0, 0, 0.25, 0.1)
                    - ball.disruptiveness();
            if (disruptiveness < 0) disruptiveness = -disruptiveness;

            //距离
            double distance = deviation(4.5, 4.5, ball.power(), ball.disruptiveness());

            BallType type = (power < 0 || disruptiveness < 0) ? BallType.OUT_BALL : BallType.PASS;

            //体力减少
            staminaChange(1 - (ball.power() - power) / 10000);

            return new Ball(power, disruptiveness, distance, type);
        }
    }

    //拦网
    public Ball block(Ball ball) {
        //质量参数
        double quality = getBlock();

        //触球
        double abilityFactor = (quality * 0.3
                + getAbility(0.2, 0, 0.3, 0.2))
                * getHeight() / 100;
        double challengeFactor = ball.power() * (1 - getFlexibility() / 200) * 0.7 + ball.disruptiveness() * 0.3;
        boolean isBlock = 1 / (1 + pow(challengeFactor, 2) / pow(abilityFactor, 2))
                > random.nextDouble();

        if (!isBlock) return ball;

        //拦回
        double controlFactor = quality * 0.15
                + getAbility(0.25, 0.3, 0.15, 0.15)
                - ball.power() * 0.7 - ball.disruptiveness() * 0.3;

        BallType type = controlFactor > 0 ? BallType.BLOCK : BallType.TOUCH;

        //威力
        double power = ((100 - controlFactor) * ball.power() + (100 + controlFactor)
                * getAbility(0.5, 0.5, 0, 0)) / 200;

        //破坏性
        double disruptiveness = ((100 - controlFactor) * ball.disruptiveness() + (100 + controlFactor)
                * (quality * 0.4 + getAbility(0, 0, 0.3, 0.3)))/ 200;

        //距离
        double distance = deviation(4, 2, power, disruptiveness);

        if (distance < 0) type = BallType.OUT_BALL;

        //体力减少
        staminaChange(1 - power / 10000);

        return new Ball(power, disruptiveness, distance, type);
    }

    //体力变化
    public void staminaChange(double factor) {
        if (factor < 1) { //体力下降
            factor = 1 - (1 - factor) * (1 - this.stamina_resist / 100.0);
            this.stamina *= factor;
        } else { //体力恢复
            factor = 1 + (factor - 1) * (1 + this.mentality_resist / 100.0);
            this.stamina *= factor;
            if (this.stamina > 100) this.stamina = 100;
        }
    }

    //心态变化
    public void mentalityChange(double factor) {
        if (factor < 1) { //心态下降
            factor = 1 - (1 - factor) * (1 - this.mentality_resist / 100.0);
            this.mentality *= factor;
        } else { //心态恢复
            factor = 1 + (factor - 1) * (1 + this.mentality_resist / 100.0);
            this.mentality *= factor;
            if (this.mentality > 100) this.mentality = 100;
        }
    }

    //重置体力和心态
    public void prepare() {
        this.stamina = 100.0;
        this.mentality = 100.0;
    }

    //能力波动
    static class NormalDistribution {
        Random random = new Random();
        public double get(double mean, double standardDeviation) {
            return mean + standardDeviation * random.nextGaussian();
        }
    }

    private double fluctuation(double mean) {
        double value = normalDistribution.get(mean,
                10 * (100.0 / reliable) * (100.0 / stamina) * (100.0 / mentality));
        return value < 0 ? 0 : value > 100 ? 100 : value;
    }

    private double getServe() {
        return fluctuation(serve);
    }

    private double getPass() {
        return fluctuation(pass);
    }

    private double getSet() {
        return fluctuation(set);
    }

    private double getSpike() {
        return fluctuation(spike);
    }

    private double getBlock() {
        return fluctuation(block);
    }

    private double getAbility(double strengthWeight, double heightWeight, double flexibilityWeight, double reliableWeight) {
        return getStrength() * strengthWeight
                + getHeight() * heightWeight
                + getFlexibility() * flexibilityWeight
                + getReliable() * reliableWeight;
    }

    private double getStrength() {
        return fluctuation(strength);
    }

    private double getHeight() {
        return fluctuation(height);
    }

    private double getFlexibility() {
        return fluctuation(flexibility);
    }

    private double getReliable() {
        return fluctuation(reliable);
    }

    //位置偏移
    private double deviation(double mean, double standardDeviation, double power, double disruptiveness) {
        return normalDistribution.get(mean,
                standardDeviation
                        * ((power * 0.7 + disruptiveness * 0.3) / 100.0)
                        * (100.0 / reliable) * (100.0 / reliable)
                        * (100.0 / stamina) * (100.0 / mentality));
    }
}