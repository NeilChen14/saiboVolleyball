package com.example.entity;

import com.example.group.Role;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.util.ArrayList;

public class PlayerTest {
    static ArrayList<Player> players = new ArrayList<>();

    @BeforeClass
    public static void init() {
        for (int i = 0; i <= 100; i += 10) {
            players.add(new Player(("队员" + i),
                    i, i, i, i, i,
                    i, i, i, i,
                    i, i));
        }
    }

    @Before
    public void prepare() {
        for (Player player : players) {
            player.prepare();
        }
    }

    @Test
    public void serveTest1() {
        Player player = players.get(8);

        System.out.println(player.getName() + "发球：");
        for (int i = 0; i < 20; i++) {
            Ball ball = player.serve();
            System.out.println(ball);
        }
    }

    @Test
    public void serveTest2() {
        Player player = players.get(8);

        int down = 0;
        int out = 0;
        int count = 100000;
        System.out.println(player.getName() + "发球：");
        for (int i = 0; i < count; i++) {
            Ball ball = player.serve();
            if (ball.distance() < 0) down++;
            if (ball.distance() > 9) out++;
            player.prepare();
        }
        System.out.println("下网：" + down * 100 / (double)count + "%");
        System.out.println("出界：" + out * 100 / (double)count + "%");
    }

    @Test
    public void passTest1() {
        Player server = players.get(8);
        Player receiver = players.get(7);

        System.out.println(server.getName() + "向" + receiver.getName() + "发球：");
        for (int i = 1; i <= 20; i++) {
            System.out.println("第" + i + "球：");
            Ball ball = server.serve();
            System.out.println(ball);
            ball = receiver.pass(ball);
            System.out.println(ball);
        }
    }

    @Test
    public void passTest2() {
        Player server = players.get(8);
        Player receiver = players.get(7);

        int fail = 0;
        int count = 100000;
        int over = 0;
        System.out.println(server.getName() + "向" + receiver.getName() + "发球：");
        for (int i = 1; i <= count; i++) {
            Ball ball = server.serve();
            if (ball.distance() > 0 && ball.distance() < 9) {
                over++;
                ball = receiver.pass(ball);
                if (ball.type() == BallType.FAILED_PASS) fail++;
            }
            server.prepare();
            receiver.prepare();
        }
        System.out.println(receiver.getName() + "接飞率：" + ((int) (fail * 10000.0 / over)) / 100.0 + "%");
    }

    @Test
    public void passTest3() {
        Player server = players.get(8);
        Player receiver = players.get(8);

        System.out.println(server.getName() + "向" + receiver.getName() + "发球：");
        for (int i = 1; i <= 10; i++) {
            System.out.println("第" + i + "球：");
            Ball ball = server.serve();
            System.out.println("发球威力：" + (int)ball.power() + " 破坏性：" + (int)ball.disruptiveness());
            ball = receiver.pass(ball);
            System.out.println("接球威力：" + (int)ball.power() + " 破坏性：" + (int)ball.disruptiveness());
        }
    }

    @Test
    public void passTest4() {
        Player server = players.get(8);
        Player receiver = players.get(7);

        int bad = 0;
        int over = 0;
        int count = 10000;
        System.out.println(server.getName() + "向" + receiver.getName() + "发球：");
        for (int i = 1; i <= count; i++) {
            Ball ball = server.serve();
            ball = receiver.pass(ball);
            if (ball.distance() < 0) over++;
            if (ball.distance() > 6) bad++;
            server.prepare();
            receiver.prepare();
        }
        System.out.println("bad:" + bad/(double)count);
        System.out.println("over:" + over/(double)count);
    }

    @Test
    public void setTest1() {
        Player setter = players.get(8);

        System.out.println(setter.getName() + "传球：");
        for (int i = 1; i <= 20; i++) {
            System.out.println("第" + i + "球：");
            Ball ball = setter.set(new Ball(30, 30, 3, BallType.PASS), 2);
            System.out.println(ball);
        }
    }

    @Test
    public void spikeTest1() {
        Player spiker = players.get(8);

        System.out.println(spiker.getName() + "扣球：");
        for (int i = 1; i <= 20; i++) {
            System.out.println("第" + i + "球：");
            Ball ball = spiker.spike(new Ball(20, 20, 2, BallType.SET), 2, Role.MB);
            System.out.println(ball);
        }
    }

    @Test
    public void spikeTest2() {
        Player spiker = players.get(8);

        System.out.println(spiker.getName() + "处理球：");
        for (int i = 1; i <= 20; i++) {
            System.out.println("第" + i + "球：");
            Ball ball = spiker.spike(new Ball(40, 40, 2, BallType.BAD_PASS), 2, Role.MB);
            System.out.println(ball);
        }
    }

    @Test
    public void blockTest1() {
        Player blocker = players.get(8);

        System.out.println(blocker.getName() + "拦网：");
        for (int i = 1; i <= 100; i++) {
            System.out.println("第" + i + "球：");
            Ball ball = blocker.block(new Ball(80, 80, 5, BallType.SPIKE));
            System.out.println(ball);
        }
    }

    @Test
    public void blockTest2() {
        Player blocker = players.get(8);

        System.out.println(blocker.getName() + "拦网：");
        int count = 100000;
        int block = 0;
        int out = 0;
        int touch = 0;
        int destroy = 0;
        int none = 0;
        for (int i = 1; i <= count; i++) {
            Ball ball = blocker.block(new Ball(80, 80, 5, BallType.SPIKE));
            switch (ball.type()) {
                case BLOCK:
                    block++;
                    break;
                case TOUCH:
                    if (ball.power() < 80) touch++;
                    else destroy++;
                    break;
                case OUT_BALL:
                    out++;
                    break;
                default:
                    none++;
                    break;
            }
            blocker.prepare();
        }
        System.out.println("拦截：" + block * 100 / (double)count + "%");
        System.out.println("撑起：" + touch * 100 / (double)count + "%");
        System.out.println("破坏：" + destroy * 100 / (double)count + "%");
        System.out.println("出界：" + out * 100 / (double)count + "%");
        System.out.println("未碰球：" + none * 100 / (double)count + "%");
    }
}
