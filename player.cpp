#include "player.h"
#include "config.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <algorithm>

std::vector<Player> allPlayers;
Player teamA[7], teamB[7];
bool used[1000];

// 添加去除字符串前后空格的函数
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

void inputPlayerData() {
    Player newPlayer;

    // 基础信息录入
    std::cout << "===== 录入球员基础信息 =====" << std::endl;
    std::cout << "请输入球员姓名：";
    std::cin >> newPlayer.name;
    std::cout << "请输入球员位置（如主攻/副攻/二传/自由人）：";
    std::cin >> newPlayer.position;
    std::cout << "请输入球员性别（0为女，1为男）：";
    std::cin >> newPlayer.gender;

    // 技术维度录入
    // 对五项能力值应用映射函数
    std::cout << "\n===== 录入扣球属性（0-100） =====" << std::endl;
    int tempSpike;
    std::cin >> tempSpike;
    newPlayer.spike = mapAbilityValue(tempSpike);
    std::cout << "\n===== 录入拦网属性（0-100） =====" << std::endl;
    int tempBlock;
    std::cin >> tempBlock;
    newPlayer.block = mapAbilityValue(tempBlock);
    std::cout << "\n===== 录入发球属性（0-100） =====" << std::endl;
    int tempServe;
    std::cin >> tempServe;
    newPlayer.serve = mapAbilityValue(tempServe);
    std::cout << "\n===== 录入传球属性（0-100） =====" << std::endl;
    int tempPass;
    std::cin >> tempPass;
    newPlayer.pass = mapAbilityValue(tempPass);
    std::cout << "\n===== 录入防守属性（0-100） =====" << std::endl;
    int tempDefense;
    std::cin >> tempDefense;
    newPlayer.defense = mapAbilityValue(tempDefense);
    std::cout << "\n===== 录入调整属性（0-100） =====" << std::endl;
    std::cin >> newPlayer.adjust;

    // 身体素质属性
    std::cout << "\n===== 录入身体耐力素质属性 =====" << std::endl;
    std::cin >> newPlayer.stamina;

    // 心理素质属性
    std::cout << "\n===== 录入心理素质属性（0-100） =====" << std::endl;
    std::cout << "抗压能力：";
    std::cin >> newPlayer.mental.pressureResist;
    std::cout << "专注度：";
    std::cin >> newPlayer.mental.concentration;
    std::cout << "自信心：";
    std::cin >> newPlayer.mental.confidence;
    std::cout << "沟通与团队协作能力：";
    std::cin >> newPlayer.mental.commu_and_teamwork;
    std::cout << "团队压力（负面属性）：";
    std::cin >> newPlayer.mental.teampressure;

    // 将球员数据写入txt文件（追加模式）
    std::ofstream ofs("players.txt", std::ios::app);
    if (!ofs.is_open()) {
        std::cerr << "无法打开文件进行写入！" << std::endl;
        return;
    }

    // 按固定格式写入（逗号分隔，便于后续读取）
    ofs << newPlayer.name << ","
        << newPlayer.position << ","
        << newPlayer.gender << ","
        << newPlayer.spike << ","
        << newPlayer.block << ","
        << newPlayer.serve << ","
        << newPlayer.pass << ","
        << newPlayer.defense << ","
        << newPlayer.adjust << ","
        << newPlayer.stamina << ","
        << newPlayer.mental.pressureResist << ","
        << newPlayer.mental.concentration << ","
        << newPlayer.mental.confidence << ","
        << newPlayer.mental.commu_and_teamwork << ","
        << newPlayer.mental.teampressure << "\n";
    ofs.close();

    std::cout << "\n球员数据录入成功！已保存至players.txt" << std::endl;
    readData();
    system("pause");
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void readData() {
    std::ifstream ifs("players.txt");
    if (!ifs.is_open()) {
        std::cerr << "无法打开文件进行读取！" << std::endl;
        return;
    }

    Player aNewPlayer;
    std::string line;
    allPlayers.clear();
    while(std::getline(ifs, line)) {
        // 跳过注释行（以"//"开头的行）和空行
        if (line.empty() || line.find("//") == 0) {
            continue;
        }

        std::vector<std::string> fields = split(line, ',');

        if(fields.size() < 15) continue;

        aNewPlayer.name = trim(fields[0]);
        aNewPlayer.position = trim(fields[1]);
        aNewPlayer.gender = std::stoi(fields[2]);
        // 对五项能力值应用映射函数
        aNewPlayer.spike = mapAbilityValue(std::stoi(fields[3]));
        aNewPlayer.block = mapAbilityValue(std::stoi(fields[4]));
        aNewPlayer.serve = mapAbilityValue(std::stoi(fields[5]));
        aNewPlayer.pass = mapAbilityValue(std::stoi(fields[6]));
        aNewPlayer.defense = mapAbilityValue(std::stoi(fields[7]));


        aNewPlayer.adjust = std::stoi(fields[8]);
        aNewPlayer.stamina = std::stoi(fields[9]);
        aNewPlayer.mental.pressureResist = std::stoi(fields[10]);
        aNewPlayer.mental.concentration = std::stoi(fields[11]);
        aNewPlayer.mental.confidence = std::stoi(fields[12]);
        aNewPlayer.mental.commu_and_teamwork = std::stoi(fields[13]);
        aNewPlayer.mental.teampressure = std::stoi(fields[14]);

        allPlayers.push_back(aNewPlayer);
    }

    ifs.close();
}

void inputPlayerByPreset() {
    teamA[0] = allPlayers[0];
    teamA[1] = allPlayers[1];
    teamA[2] = allPlayers[2];
    teamA[3] = allPlayers[3];
    teamA[4] = allPlayers[4];
    teamA[5] = allPlayers[5];
    teamA[6] = allPlayers[6];
    teamB[0] = allPlayers[7];
    teamB[1] = allPlayers[8];
    teamB[2] = allPlayers[9];
    teamB[3] = allPlayers[10];
    teamB[4] = allPlayers[11];
    teamB[5] = allPlayers[12];
    teamB[6] = allPlayers[13];
}


void inputPlayer() {

#if USE_PRESET_INPUT
     inputPlayerByPreset();
     return;
#endif



    std::string playerName;
    memset(used, 0, sizeof(used));
    printf("\n\n-------------\n");
    printf("请输入A队球员（全名）：\n");
    for(int i = 0; i < 6; i++) {
        printf("%d号位球员姓名：", i + 1);
        std::cin >> playerName;
        int chosen = -1;
        for(int j = 0; j < allPlayers.size(); j++) {
            if(allPlayers[j].name == playerName) {
                if(used[j]) {
                    std::cout << playerName << "已被使用。\n";
                    chosen = -2;
                    break;
                }
                chosen = j;
                break;
            }
        }
        if(chosen == -2) {
            i--;
            continue;
        }
        if(chosen == -1) {
            std::cout << "未找到" << playerName << "，请重新确认名字。\n";
            i--;
            continue;
        }

        std::cout << "A队" << i + 1 << "号位选择：" << allPlayers[chosen].name << "|" << allPlayers[chosen].position << "\n";
        teamA[i] = allPlayers[chosen];
        used[chosen] = true;
    }

    // 输入A队自由人
    printf("请输入A队自由人姓名：");
    std::cin >> playerName;
    int chosenLiberoA = -1;
    for(int j = 0; j < allPlayers.size(); j++) {
        if(allPlayers[j].name == playerName) {
            if(used[j]) { std::cout << playerName << "已被使用。\n"; chosenLiberoA = -2; break; }
            chosenLiberoA = j;
            break;
        }
    }
    while(chosenLiberoA == -1 || chosenLiberoA == -2) {
        if(chosenLiberoA == -2) { chosenLiberoA = -1; }
        std::cout << "未找到自由人，请重新输入：";
        std::cin >> playerName;
        for(int j = 0; j < allPlayers.size(); j++) {
            if(allPlayers[j].name == playerName && !used[j]) {
                chosenLiberoA = j;
                break;
            }
        }
    }
    teamA[6] = allPlayers[chosenLiberoA];
    used[chosenLiberoA] = true;

    printf("\n\n-------------\n");
    printf("请输入B队球员（全名）：\n");
    for(int i = 0; i < 6; i++) {
        printf("%d号位球员姓名：", i + 1);
        std::cin >> playerName;
        int chosen = -1;
        for(int j = 0; j < allPlayers.size(); j++) {
            if(allPlayers[j].name == playerName) {
                if(used[j]) {
                    std::cout << playerName << "已被使用。\n";
                    chosen = -2;
                    break;
                }
                chosen = j;
                break;
            }
        }
        if(chosen == -2) {
            i--;
            continue;
        }
        if(chosen == -1) {
            std::cout << "未找到" << playerName << "，请重新确认名字。\n";
            i--;
            continue;
        }
        teamB[i] = allPlayers[chosen];
        used[chosen] = true;
    }

    // 输入B队自由人
    printf("请输入B队自由人姓名：");
    std::cin >> playerName;
    int chosenLiberoB = -1;
    for(int j = 0; j < allPlayers.size(); j++) {
        if(allPlayers[j].name == playerName) {
            if(used[j]) { std::cout << playerName << "已被使用。\n"; chosenLiberoB = -2; break; }
            chosenLiberoB = j;
            break;
        }
    }
    while(chosenLiberoB == -1 || chosenLiberoB == -2) {
        if(chosenLiberoB == -2) { chosenLiberoB = -1; }
        std::cout << "未找到自由人，请重新输入：";
        std::cin >> playerName;
        for(int j = 0; j < allPlayers.size(); j++) {
            if(allPlayers[j].name == playerName && !used[j]) {
                chosenLiberoB = j;
                break;
            }
        }
    }
    teamB[6] = allPlayers[chosenLiberoB];
    used[chosenLiberoB] = true;
}

void showAllPlayer() {
    printf("\n\n-------------\n");
    for(int i = 0; i < allPlayers.size(); i++) {
        std::cout << allPlayers[i].name << "|" << allPlayers[i].position << "|" << (allPlayers[i].gender ? "男" : "女") << "\n";
        printf("扣球:%d|拦网:%d|发球:%d|传球:%d|防守:%d|调整:%d\n", allPlayers[i].spike, allPlayers[i].block, allPlayers[i].serve, allPlayers[i].pass, allPlayers[i].defense, allPlayers[i].adjust);
        printf("耐力:%d|抗压:%d|专注:%d|自信:%d|团队:%d|压力:%d\n", allPlayers[i].stamina, allPlayers[i].mental.pressureResist, allPlayers[i].mental.concentration, allPlayers[i].mental.confidence, allPlayers[i].mental.commu_and_teamwork, allPlayers[i].mental.teampressure);
        printf("-------------\n");
    }
    system("pause");
}

bool isPosition(const Player& player, std::string pos) {
    return pos == player.position;
}
