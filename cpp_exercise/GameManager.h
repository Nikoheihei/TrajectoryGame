#pragma once
#include "GameObject.h"
#include "Player.h"
#include <vector>
#include <string>
#include <fstream>

class GameManager {
public:
    // 游戏模式枚举
    enum GameMode { 
        SIMPLE_SINGLE,  // 简单单人模式
        COMPLEX_SINGLE, // 复杂单人模式
        SIMPLE_MULTI,   // 简单多人模式
        COMPLEX_MULTI,  // 复杂多人模式
        TIME_BASED_MODE, // 计时模式
    };

private:
    GameObject objectA;
    GameObject sharedObjectA; // 用于在多人模式下共享轨迹数据
    std::vector<Player> players;
    int currentPlayerIndex;
    GameMode currentGameMode;
    std::string loggedInUsername;
    bool isLoggedIn; // 添加登录状态标志
    
    // 游戏配置
    int gameSteps;      // 轨迹的步数
    bool gameRunning;   // 游戏运行状态
    int currentRound;   // 当前回合数
    int totalRounds;    // 总回合数

    // 轨迹步数范围配置
    static const int SIMPLE_STEPS = 10;   
    static const int HARD_STEPS = 10;  

    std::string userInfoFile = "userInfor.txt";
    std::string doubleTempFile = "doubletemp.txt";
    std::string doublePlayerFile = "doubleplayer.txt"; 
    
    // 新增步数限制
    static const int SIMPLE_MODE_MAX_STEPS = 20;
    static const int COMPLEX_MODE_MAX_STEPS = 10;
    static const int TIME_MODE_STEPS = 5;
    
    // 新增计时相关配置
    const int TIMED_MODE_ROUNDS = 2;
    
    // 新增方法
    void updateUserStats(const std::string& username, bool isComplexMode, bool isWin);

public:
    // 构造函数
    GameManager();
    
    // 初始化游戏
    void initializeGame(GameMode mode, std::string username1, std::string username2);
    
    // 从文件加载用户信息
    bool CheakIfUserExist(std::fstream& userInfoFile, std::string username);
    
    // 生成游戏数据（A和B的轨迹等）
    void generateGameData();

    // 设置轨迹步数
    void setGameSteps(int steps);
    
    // 获取轨迹步数
    int getGameSteps() const {
        return gameSteps;
    }
    
    // 处理用户输入
    void handleUserInput();
    
    // 更新游戏状态
    void updateGame();
    
    //更新总轮次
    void updateTotalRounds(int rounds);
    
    // 切换当前玩家（多人模式）
    void switchPlayer();
    
    // 将用户信息保存到文件
    void saveUserData();
    
    // 注册新用户
    bool registerUser(std::fstream& userInfoFile, const std::string& username);
    
    // 用户登录
    bool loginUser(std::fstream&userInfoFile,const std::string& username);
    
    // 获取当前游戏模式
    GameMode getCurrentGameMode() const;
    
    // 获取当前玩家
    Player& getCurrentPlayer();
    //获取指定编号的玩家
    Player& getPlayer(int index);
    
    // 获取ObjectA
    const GameObject& getObjectA() const;
    
    
    // 检查游戏是否在运行
    bool isGameRunning() const;
    
    // 判断是否为复杂模式
    bool isComplexMode() const {
        return currentGameMode == COMPLEX_SINGLE || currentGameMode == COMPLEX_MULTI;
    }
    
    // 判断是否为多人模式
    bool isMultiplayerMode() const {
        return currentGameMode == SIMPLE_MULTI || currentGameMode == COMPLEX_MULTI || currentGameMode == TIME_BASED_MODE;
    }
    
    // 获取当前回合数
    int getCurrentRound() const {
        return currentRound;
    }
    
    // 获取总回合数
    int getTotalRounds() const {
        return totalRounds;
    }
    
    // 检查游戏是否结束
    bool isGameOver() const;
    
    // 开始新回合
    void startNewRound();
    
    // 获取所有玩家
    const std::vector<Player>& getPlayers() const {
        return players;
    }

    void setGameMode(int mode);
    int getMaxStepsForMode() const;
    bool isTimeBasedGame() const { return currentGameMode == TIME_BASED_MODE; }
    void resetRound();
    
    // 保存时间结果
    void saveTimeResult(const std::string& player1, const std::string& player2, double time1, double time2);

    void continueMultiplayerGame();
    void saveDoublePlayerResult(std::fstream&filename,const std::string& player1, const std::string& player2);
    
    // 用户登录状态管理
    bool isUserLoggedIn() const {
        return isLoggedIn;
    }
    
    void logout() {
        isLoggedIn = false;
        loggedInUsername = "";
    }
    
    const std::string& getLoggedInUsername() const {
        return loggedInUsername;
    }
}; 