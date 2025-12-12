---
layout: default  
title: "saiboVolleyball项目说明"
---


## 实现功能

### 1.球员读入

### 2.比赛模拟

按照发球 -> 接一 -> 二传 -> 扣球 -> 拦网 -> 防守 -> 二传的步骤循环进行

**发球：**  
包含不同的发球策略：冲发球、稳定发球。
球员根据自身发球水平、信心值等数据进行决策。
发球结果根据球员发球属性以及其他心理属性（如抗压、专注度等）决定，亦有随机因素影响。

**接一：**  
目前当且仅当接应在3号位时，后排3人接一，否则始终为4人接一（双主攻+自由人+接应）。
除了后排接一球员，亦有小概率前排副攻接一（弹网球）。
根据球员的防守属性以及其他属性决定一传质量。

**二传：**  
二传根据球的到位程度，各进攻球员的下球率来决策传球目标。
二传的传球质量根据一传质量、二传本身的二传属性以及其他心理属性和随机因素影响。
亦包含二次的选项。

**扣球：**  
扣球球员根据二传质量等各方面因素，决定扣球策略，包含强攻、避手、快球、吊球、调整、处理等。
扣球球员的扣球强度根据扣球策略、二传质量、自身扣球水平、以及其他属性影响。
根据上述属性，亦会产生一个拦网值，代表扣球是否容易被拦。
后排进攻会有扣球强度和拦网值补正。

**拦网：**  
根据对方的进攻类型，进行拦网，包括三人拦网、双人拦网、单人拦网等。其中根据特殊规则，对方女生进攻男生不得拦网。
根据所有拦网人的拦网属性、团队协作属性与其他属性来决定一个拦网值。最终决定一个拦网结果，包括拦回、撑起、无接触、破坏拦网等。
其中拦回则直接回到对方的防守轮次。
不同人数拦网会有一个最终的参数补正。人数越多强度越高。

**防守：**  
随机指定不参与拦网的防守球员，进行防守检定。
根据拦网后的进攻强度（撑起会降低、破坏拦网会提高）、球员防守属性、以及其他属性决定防守结果。

## 备注
目前只实现了单自由人，且必定有自由人，且单自由人必定替换双副攻。

输入请确保包含 1个二传1个接应 2副攻 2主攻，且确保二传接应对位，副攻对位，主攻对位。

对已有球员的位置/数值不满意可以去 /cmake-build-debug/players.txt 里面直接修改。

你可以打开config.h来对部分比赛过程中的参数进行调整，或开启调试模式。

你可以将config.h中的 USE_PRESET_INPUT 属性值设置为1，以使用预设输入：
此情况下，程序将自动读取players.txt中1-7位作为A队1-6号位&自由人，8-14位为B队1-6为&自由人。

目前的players.txt已经按正确格式预设了两支队伍。

你可以在config.h中手动设置PRE_SEED作为随机数种子，来实现两次完全一样的比赛。若未设置，则随机生成的种子将会保存在/cmake-build-debug/seeds.txt的最新一行。

## 详细数值计算细节

输入数据时，对于发球、扣球、拦网、传球和防守五大属性，进行分散调整，调整细节如下：

$$
x =
\begin{cases}
0.4x & x \leq 50 \\
1.5x-55 & 50 < x \leq 90 \\
2x-100 & x > 90
\end{cases}
$$

### 一、发球部分

**1.发球决策**  
$$
\begin{aligned}
Q&=(SRV/100 \times 0.75 + CFD/100 \times 0.25)\times0.4 \\
&+CON/100\times0.2 \\
&+SIT\times0.2 \\
&+(1-PRS/100)\times0.1 \\
&+STM/100\times(1-(matchNum-1)\times0.2)\times0.1 \\
&+(rand()\%20-10) / 100 \\
\end{aligned}
$$

其中$SRV$为发球属性、$CFD$为自信属性、$CON$为专注属性、$PRS$为抗压属性、$STM$为耐力属性。  
$SIT$为局势参数，当领先大于2分时，$SIT$为$0.3$，落后大于2分时，$SIT$为$0.7$，否则为$0.5$。  
$matchNum$为比赛局数。

决策结果：
$$
result=
\begin{cases}
aggresive\_serve & Q > AGGRESSIVE\_SERVE\_THRESHOLD \\
stable\_serve & Q \leq AGGRESSIVE\_SERVE\_THRESHOLD
\end{cases}
$$
其中$AGGRESSIVE\_SERVE\_THRESHOLD$为发球决策阈值，可在config.h中调整。

**2.发球强度计算：**

影响系数$adj$
$$
\begin{aligned}
adj&=1.0 \times \sqrt[4]{(STM/100)} \times (1.0-(matchNum-1)\times 0.1) \\ 
&\times(0.85+0.3\times PRS / 100) \\ 
&\times (0.9 + 0.2\times CON / 100) \\
&\times (1.0+(rand()\%20-10) / 100)
\end{aligned}
$$

$$adj=\max(adj,0.3)$$

发球强度$power$

$$
power=
\begin{cases}
SEV\times BASIC\_POWER \times(0.8+0.2\times adj) & stable\_serve \\
SEV\times BASIC\_POWER \times adj & aggressive\_serve
\end{cases}
$$

其中
$$
BASIC\_POWER=
\begin{cases}
0.7 & stable\_serve \\
1.2 & aggressive\_serve
\end{cases}
$$
$BASIC\_POWER$数值可在config.h中调整

**3.发球失误率计算**

$$fault=BASIC\_FAULT \times (2.0-adj) \times (SEV / 100) \times 0.5$$
$$fault=\max(0.05, \min(0.8, fault))$$
其中
$$
BASIC\_FAULT=
\begin{cases}
0.15 & stable\_serve \\
0.3 & aggressive\_serve
\end{cases}
$$
$BASIC\_FAULT$数值可在config.h中调整

### 二、接一部分

**1.接一球员选择**

当且仅当接应在3号位时，后排三人接一，否则为后排四人接一。三人接一为双主攻+自由人，四人接一为双主攻+自由人+接应。

90%概率由后排接一的随机一名球员接一；  
10%概率由前排副攻接一。

**2.接一成功率计算**

影响系数$adj$
$$
\begin{aligned}
adj&=1.0 \times \sqrt[4]{(STM/100)} \times (1.0-(matchNum-1)\times 0.1) \\
&\times(0.85+0.3\times PRS / 100) \\
&\times (0.9 + 0.2\times CON / 100) \\
&\times (0.9 + 0.2\times TMW / 100) \\
&\times (1.0+(rand()\%20-10) / 100)
\end{aligned}
$$

$$adj=\max(adj,0.3)$$

其中，$TMW$为团队协助属性。

接一成功率$succ$
$$
\begin{aligned}
succ&=DEF \times (1 + 0.4 \times adj) / 100 \\
&\times(1 - power / 100) \\
&+ (rand()\% 5 - 10) / 100 \\
\end{aligned}
$$

$$succ = \max(0, \min(1, succ))$$

其中，$DEF$为防守属性，$power$为发球强度。

**3.接一结果**
$$
result=
\begin{cases}
perfect & random < succ\times 0.2 \\
good & succ\times 0.2 \leq random < succ\times 0.7 \\
bad & succ\times 0.7 \leq random < succ \\
fault & succ \leq random
\end{cases}
$$

接一质量值$qua$
$$
qua=
\begin{cases}
90 + rand() \% 11 & perfect \\
70 + rand() \% 20 & good \\
40 + rand() \% 30 & bad \\
0 & fault
\end{cases}
$$

### 三、二传部分

**1.决策传球对象**

（1）当一传质量为$perfect$时： 

在前排主攻、后排主攻、前排副攻、接应中，排序选取进攻效果值$eff$最高的。

$$
eff=blockBonus + positionBonus + 0.1\times STM
$$

其中$blockBonus$的计算方式为：  
寻找进攻者对应的对方单个拦网人（前排攻手为对方对应位置拦网人，后排主攻进攻为对方前排副攻拦网，后排接应进攻为对方前排主攻拦网）；  
$blockBonus=(\max(-1,\min(1,(SPK-BLK)/100)) + 1)\times27.5$  
其中$SPK$为传球目标扣球属性，$BLK$为对应拦网人拦网属性  
特别的，进攻者为女生且对应拦网者为男生，则$blockBonus$记为最大值$55$  

其中$positionBonus$的计算方式为：
$$
positionBonus=
\begin{cases}
35 & 前排副攻或前排接应 \\
30 & 前排主攻 \\
12 & 后排主攻 \\
10 & 后排接应 \\
\end{cases}
$$
特别的，若选的是副攻，副攻最终的$eff$乘以$1.2$

（2）当一传质量为$good$时：

若接应在前排：$55\%$概率前排主攻，$35\%$概率前排接应，$10\%$概率后排主攻，$10\%$概率二次。

若接应在后排：$70\%$概率前排主攻，$10\%$概率后排接应，$10\%$概率后排主攻，$10\%$概率二次。

（3）当一传质量为$bad$时：

若接应在前排：$40\%$概率调整攻（主攻），$30\%$概率前排接应，$10\%$概率后排主攻，$20\%$概率二次。

若接应在后排：$60\%$概率调整攻（主攻），$10\%$概率后排接应，$10\%$概率后排主攻，$20\%$概率二次。

**2.计算传球属性值**

传球影响系数$adj$
$$
\begin{aligned}
adj&=1.0 \times \sqrt[4]{(STM/100)} \times (1.0-(matchNum-1)\times 0.1) \\
&\times(0.85+0.3\times PRS / 100) \\
&\times (0.85 + 0.3\times CON / 100) \\
&\times (0.9 + 0.2\times TMW / 100) \\
&\times (1.0+(0.3+(100-qua)\times 0.4)\times (ADJ / 100))
\end{aligned}
$$

$$adj=\max(adj,0.3)$$
其中$qua$为一传质量，$ADJ$为二传调整属性。

传球质量值$passQua$为：

$$passQua=PAS\times adj\times \sqrt{qua/100} \ / \  diffFactor + (rand()\%20 - 10)$$

$$passQua=\lfloor \max(0, passQua) \rfloor$$

其中$PAS$为二传传球属性，$diffFactor$为：

$$
diffFactor=
\begin{cases}
1.4 & 前排副攻 \\
1.4 & 后排主攻 \\
1.2 & 接应 \\
1.0 & 前排主攻(非调整攻) \\
0.8 & 调整攻
\end{cases}
$$

$passQua$小于$50$时，视为传球失误，直接失分。

**3.计算二次进攻属性**

（1）决策二次进攻类型  

$$
result=
\begin{cases}
扣球 & (rand() \% 100 / 100) < spike /(spike+tip) \\
吊球 & (rand() \% 100 / 100) \geq spike /(spike+tip)
\end{cases}
$$

其中：
$$spike=(SPK \times 0.7 + PRS\times0.3)\times0.7$$
$$tip = PAS\times 0.6 + WIS\times0.4$$
其中$SPK$为二传扣球属性，$WIS$为二传球商属性。

（2）计算二次进攻强度

### 四、扣球部分

### 五、拦网部分

### 六、防守部分

## 更新日志

#### 25.12.2 by yaorz26
~~~
实现了基础的球员读入与比赛模拟功能。
~~~

#### 25.12.5 by NeilChen14
~~~
重新设计了球员的基础数值；
新增了球员数值的映射函数，使球员数值分布更合理；
调整了部分基础参数；
降低三人拦网出现概率；
修正拦网队员选择；
新增一传到位时，二传会先判断下球率再选择攻手；
修改x号位的描述，使之与现实对应；
降低对高强度扣球的防守能力以贴近现实；
新增了破坏拦网的实现；
对项目其余部分进行调整，以更贴近实际表现。
~~~

#### 25.12.11 by yaorz26
~~~
修正了256456提出的问题
将发球和接发部分的代码和其他部分的代码一同实现了面向对象化
增添了README.md
增添了随机数种子控制
调整了部分细节运算，以保持各部分一致
修正了一些已知的显示错误
~~~
