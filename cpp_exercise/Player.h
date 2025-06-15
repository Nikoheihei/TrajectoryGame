#pragma once
#include "Trajectory.h"
#include <chrono>
#include <iostream>
#include <string>

class Player {
private:
    std::string name;
    int totalScore;
    double timeTaken;  // 玩家做出预测所用的时间（秒）
    
    // 计时器相关变量
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock>endTime;
    bool timerRunning;
    
    // 新增统计变量
    int simpleModeWins = 0;
    int simpleModeTotal = 0;
    int complexModeWins = 0;
    int complexModeTotal = 0;

public:
    // 构造函数
    Player(std::string name) 
    : name(name), totalScore(0), timeTaken(0.0), timerRunning(false) {
    // 初始化玩家对象
}
    //提交成绩
    void addScore(int score);
    // 获取玩家ID
    std::string getName() const;
    
    // 开始预测计时
    void startTimer();
    
    // 结束预测计时并返回用时（秒）
    double endTimer();
    
    // 获取当前得分
    int getTotalScore() const;
    
    
    // 获取用时
    double getTimeTaken() const;
    
    // 重置玩家数据（用于新一轮游戏）
    void reset();
    
    // 新增统计方法
    void setSimpleModeStats(int wins, int total) {
        simpleModeWins = wins;
        simpleModeTotal = total;
    }
    
    void setComplexModeStats(int wins, int total) {
        complexModeWins = wins;
        complexModeTotal = total;
    }
    
    void incrementSimpleModeWins() { simpleModeWins++; }
    void incrementSimpleModeTotal() { simpleModeTotal++; }
    void incrementComplexModeWins() { complexModeWins++; }
    void incrementComplexModeTotal() { complexModeTotal++; }
    
    int getSimpleModeWins() const { return simpleModeWins; }
    int getSimpleModeTotal() const { return simpleModeTotal; }
    int getComplexModeWins() const { return complexModeWins; }
    int getComplexModeTotal() const { return complexModeTotal; }
}; 