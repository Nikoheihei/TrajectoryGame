#include "GameManager.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include<cmath>
using namespace std;
const int step=5;

GameManager::GameManager() 
    : currentPlayerIndex(-1), currentGameMode(SIMPLE_SINGLE), 
      gameSteps(10), gameRunning(false), 
      currentRound(0), totalRounds(2), isLoggedIn(false) {
    // 初始化GameManager对象
    // 加载用户数据
}


void GameManager::initializeGame(GameMode mode, string username1, string username2) {
    // 初始化游戏并设置游戏模式
    currentGameMode = mode;
    gameRunning = true;
    currentRound = 0;
    
    // 根据游戏模式设置玩家数量
    players.clear();
    if (mode == SIMPLE_SINGLE || mode == COMPLEX_SINGLE) {
        // 单人模式只有一个玩家
        players.push_back(Player(username1));
        currentPlayerIndex = 0; // 确保单人模式下玩家索引设置为0
    } else {
        // 多人模式有两个玩家
        players.push_back(Player(username1));
        // 确保username2不为空
        if (username2.empty()) {
            username2 = "Player2"; // 提供默认名称
        }
        players.push_back(Player(username2));
        currentPlayerIndex = -1; // 多人模式下，让switchPlayer方法来设置为0
    }
    
    // 根据游戏模式设置游戏步数和回合数
    if (mode == TIME_BASED_MODE) {
        gameSteps = 10;  // 计时模式使用较少步数
        totalRounds = TIMED_MODE_ROUNDS;  // 设置计时模式回合数
    } else {
        gameSteps = 10;  // 标准模式步数
    }
    
    // 生成游戏数据
    generateGameData();
}

bool GameManager::CheakIfUserExist(fstream&userInfoFile,string username) {
    if(!userInfoFile.is_open()){
        std::cerr<<"Failed to open userInfoFile"<<std::endl;
        return false;
    }
    string line;
    while (getline(userInfoFile, line)) {
     if(line==username){
        return true;
     }
    }
    return false;
}

void GameManager::generateGameData() {
    // 生成实际轨迹和相对轨迹
    if (isMultiplayerMode() && currentPlayerIndex > 0) {
        // 在多人模式的第二个玩家时，使用和第一个玩家相同的轨迹数据
        objectA = sharedObjectA;
    } else {
        // 单人模式或多人模式第一个玩家时，生成新的轨迹
        objectA.generateTrajectory(isComplexMode(), gameSteps);
        do{objectA.generateRelativeTrajectory(gameSteps, isComplexMode());
        }while((abs(objectA.getActualTrajectory().getCell(0).getRow()-objectA.getRelativeTrajectory().getCell(0).getRow())<5)&&
        (abs(objectA.getActualTrajectory().getCell(0).getCol()-objectA.getRelativeTrajectory().getCell(0).getCol())<5));
        
        objectA.calculateActualTrajectory();
        
        // 在多人模式下，保存第一个玩家的轨迹数据供第二个玩家使用
        if (isMultiplayerMode() && currentPlayerIndex == 0) {
            sharedObjectA = objectA;
        }
    }
}


void GameManager::startNewRound() {
    // 增加回合数
    currentRound++;
    
    // 在多人模式下，重置玩家索引
    if (isMultiplayerMode()) {
        currentPlayerIndex = 0;
    }
    
    // 生成新的游戏数据
    // 这里应该总是生成新数据，因为这是开始新的一轮
    objectA.generateTrajectory(isComplexMode(), gameSteps);
    objectA.generateRelativeTrajectory(gameSteps, isComplexMode());
    objectA.calculateActualTrajectory();
    
    // 在多人模式下，保存第一个玩家的轨迹数据供后续玩家使用
    if (isMultiplayerMode()) {
        sharedObjectA = objectA;
    }
}

void GameManager::updateTotalRounds(int rounds){
    totalRounds=rounds;
}

void GameManager::switchPlayer() {
    // 切换当前玩家（多人模式）
    // 检查是否是多人模式，如果是则切换玩家
    if (isMultiplayerMode()) {
        // 只有当玩家数组不为空时才切换
        if (players.size() > 0) {
            // 如果当前玩家索引为-1，将其设为0
            if (currentPlayerIndex == -1) {
                currentPlayerIndex = 0;
            } else {
        currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
                
                // 如果切换回第一个玩家，表示进入了新的回合
                if (currentPlayerIndex == 0) {
                    currentRound++;
                }
            }
        }
    } else {
        // 单人模式直接增加回合数
        currentRound++;
    }
}

bool GameManager::registerUser(fstream&userInfoFile,const std::string& username) {
    // 注册新用户
    if(!userInfoFile.is_open()){
        cout<<"Failed to open userInfoFile"<<endl;
        return false;
    }
    
    // 检查文件是否为空
    userInfoFile.seekg(0, ios::end);
    bool isEmpty = (userInfoFile.tellg() == 0);
    
    // 如果文件不为空，检查最后一个字符是否为换行符
    if(!isEmpty) {
        userInfoFile.seekg(-1, ios::end);
        char lastChar;
        userInfoFile.get(lastChar);
        
        // 回到文件末尾准备写入
        userInfoFile.seekp(0, ios::end);
        
        // 如果最后一个字符不是换行符，先添加一个
        if(lastChar != '\n') {
            userInfoFile << endl;
        }
    }
    
    userInfoFile<<username<<endl;
    userInfoFile.close();
    return true;
}

bool GameManager::loginUser(fstream&userInfoFile,const std::string& username) {
    // 用户登录
    // 检查用户名是否存在
    if (!CheakIfUserExist(userInfoFile,username)) {
        return false; // 用户名不存在
    }
    
    // 设置当前登录用户
    loggedInUsername = username;
    isLoggedIn = true;
    return true;
}

GameManager::GameMode GameManager::getCurrentGameMode() const {
    // 返回当前游戏模式
    return currentGameMode;
}

Player& GameManager::getCurrentPlayer() {
    // 返回当前玩家
    return players[currentPlayerIndex];
}

const GameObject& GameManager::getObjectA() const {
    // 返回ObjectA
    return objectA;
}

bool GameManager::isGameRunning() const {
    // 返回游戏运行状态
    return gameRunning;
}




//void GameManager::saveTimeResult(const std::string& player1, const std::string& player2, double time1, double time2) {
//    std::ofstream file(doubleTempFile, std::ios::app);
//    file << player1 << " " << time1 << "\n"
//         << player2 << " " << time2 << "\n";
//}


bool GameManager::isGameOver() const {
    // 检查游戏是否结束
    return currentRound > totalRounds;
}

void GameManager::resetRound() {
    // 重置回合
    currentRound = 1;
    generateGameData();
}

Player& GameManager::getPlayer(int index){
    return players[index];
}

void GameManager::continueMultiplayerGame() {
    // 如果是多人模式且游戏结束，允许继续比拼
    if (isMultiplayerMode() && isGameOver()) {
        resetRound();
}
}

void GameManager::saveDoublePlayerResult(fstream&filename,const std::string& player1, const std::string& player2) {
    if(!filename.is_open()){
        cout<<"Failed to open filename"<<endl;
}
    filename<<player1<<" "<<player2<<endl;
    filename.close();
}

