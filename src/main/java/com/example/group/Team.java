package com.example.group;

import com.example.entity.Player;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

public class Team {
    private final String name;
    private final ArrayList<Player> players = new ArrayList<>();
    private final ArrayList<Player> liberos = new ArrayList<>();

    private final Map<Integer, Player> Numbers = new HashMap<>();
    private final Map<Player, Role> Roles = new HashMap<>();

    public Team(String name) {
        this.name = name;
    }

    //添加队员
    public boolean addPlayer(Player player, int number, Role role) {
        if (players.size() >= 14) return false;
        if (players.contains(player)) return false;
        if (Numbers.containsKey(number)) return false;
        if (role == Role.L) {
            if (liberos.size() >= 2) return false;
            liberos.add(player);
        }
        players.add(player);
        Numbers.put(number, player);
        Roles.put(player, role);
        return true;
    }

    public String getName() {
        return name;
    }

    public Map<Integer, Player> getPlayerList() {
        return Numbers;
    }

    public Role getRole(Player player) {
        return Roles.get(player);
    }

    public Player getPlayer(int number) {
        return Numbers.get(number);
    }
}
