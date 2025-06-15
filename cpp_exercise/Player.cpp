#include "Player.h"
#include <string>
using namespace std;


std::string Player::getName() const {
    // 返回玩家ID
    return name;
}

void Player::addScore(int score){
    // 添加得分
    totalScore += score;
}

void Player::startTimer() {
    // 开始计时
    // 记录开始时间并设置计时器状态为运行中
    startTime = std::chrono::steady_clock::now();
    timerRunning = true;
}

double Player::endTimer() {
    // 结束计时并计算用时
    // 如果计时器没有启动，返回0
    if (!timerRunning) {
        return 0.0;
    }
    
    // 计算用时（秒）
    endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;
    timeTaken = elapsed.count();
    timerRunning = false;
    
    return timeTaken;
}


int Player::getTotalScore() const {
    // 返回当前得分
    return totalScore;
}


double Player::getTimeTaken() const {
    // 返回预测用时
    return timeTaken;
}

void Player::reset() {
    // 重置玩家数据（用于新一轮游戏）
    // 保留playerID和highScore
    totalScore = 0;
    timeTaken = 0.0;
    timerRunning = false;
} 