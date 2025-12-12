package com.example.entity;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Lineup {
    private final Team team;
    private final Player[] position = new Player[7];

    private Player setter;
    private Player replacedByL;

    //替换
    private int substitutionCount = 0;
    private final Map<Player, Player> substitutionRecord = new HashMap<>();

    public Lineup(Team team, Player[] position, Player setter) {
        this.team = team;
        System.arraycopy(position, 1, this.position, 1, 6);
        this.setter = setter;
        this.replacedByL = null;
    }

    //轮转
    public void rotate() {
        //如果自由人将要轮转到前排，换下
        if (team.getRole(position[5]) == Role.L) liberoOut();
        for (int i = 1; i <= 6; i++) position[i - 1] = position[i];
        position[6] = position[0];
    }

    //换人
    public boolean substitute(Player out, Player in) {
        //次数超限
        if (substitutionCount >= 6) return false;

        //替换自由人
        if (team.getRole(in) == Role.L) return liberoIn(in, out);
        if (team.getRole(out) == Role.L) return liberoOut();

        //换下队员不在场上或换上队员已在场上
        int outPosition = 0;
        for (int i = 1; i <= 6; i++) {
            if (this.position[i] == out) outPosition = i;
            if (this.position[i] == in) return false;
        }
        if (outPosition == 0) return false;

        //替换对不匹配
        if (substitutionRecord.containsKey(out) && substitutionRecord.get(out) != in) return false;
        if (substitutionRecord.containsValue(in) && substitutionRecord.get(out) != in) return false;

        //替换二传
        if (out == setter) setter = in;

        this.position[outPosition] = in;
        substitutionRecord.put(in, out);
        substitutionCount++;
        return true;
    }

    //自由人上场
    public boolean liberoIn(Player libero, Player replacedByL) {
        //场上已有自由人
        if (this.replacedByL != null) return false;
        //换下队员不在场上
        int position = 0;
        for (int i = 1; i <= 6; i++) if (this.position[i] == replacedByL) position = i;
        if (position == 0) return false;
        //换下队员不在后排
        if (position > 1 && position < 5) return false;

        //替换二传
        if (replacedByL == setter) setter = this.position[position + 3 % 6];

        this.position[position] = libero;
        this.replacedByL = replacedByL;

        return true;
    }

    //自由人下场
    public boolean liberoOut() {
        //场上没有自由人
        if (replacedByL == null) return false;
        for (int i = 1; i <= 6; i++) if (team.getRole(this.position[i]) == Role.L) {
            position[i] = replacedByL;
            replacedByL = null;
            return true;
        }
        return false;
    }

    //替换自由人
    public boolean substituteLibero(Player newLibero) {
        if (team.getRole(newLibero) != Role.L) return false;
        for (int i = 1; i <= 6; i++) {
            if (team.getRole(this.position[i]) == Role.L) {
                this.position[i] = newLibero;
                return true;
            }
        }
        return false;
    }

    //替换二传
    public boolean substituteSetter(Player newSetter) {
        //不能为自由人
        if (team.getRole(newSetter) == Role.L) return false;
        for (int i = 1; i <= 6; i++) {
            if (this.position[i] == newSetter) {
                setter = newSetter;
                return true;
            }
        }
        //新二传不在场上
        return false;
    }

    public Player getSetter() {
        return setter;
    }

    //前排队员
    public ArrayList<Player> getFront() {
        return new ArrayList<>(Arrays.asList(position).subList(2, 5));
    }

    //后排队员
    public ArrayList<Player> getBack() {
        ArrayList<Player> back = new ArrayList<>();
        back.add(position[1]);
        back.add(position[5]);
        back.add(position[6]);
        return back;
    }
}
