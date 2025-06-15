#pragma once
#include "GridCell.h"
#include "Trajectory.h"
#include <string>
#include <vector>

class GameObject {
protected:
    Trajectory actualTrajectory; // 对象的实际移动轨迹 
    Trajectory relativeTrajectory;  // 相对轨迹
    Trajectory predictedTrajectory;  // 玩家预测的轨迹
    Trajectory finalTrajectory;

public:
    // 构造函数
    GameObject(int startRow = 0, int startCol = 0, const std::string& objectColor = "white");
    
    
    // 获取当前位置
    const GridCell& getCurrentCell(Trajectory&trajectory) const;
    //设置四个方向的移动
    void removeFourDirection(Trajectory& trajectory,const GridCell&cell,int direction);
    //设置六个方向的移动
    void removeSixDirection(Trajectory& trajectory,const GridCell&cell,int direction);
    
    // 生成实际轨迹
    void generateTrajectory(bool difficulty, int steps);
    
    // 生成相对轨迹
    void generateRelativeTrajectory(int steps, bool difficulty);
    
    // 回溯法辅助函数
    bool generateTrajectoryBacktrack(Trajectory& trajectory, int depth, int maxDepth, int lastDir, bool isComplex);
    
    // 根据参考轨迹和相对轨迹计算实际轨迹
    void calculateActualTrajectory();
    
    
    // 获取实际轨迹
    const Trajectory& getActualTrajectory() const;
    
    // 获取预测轨迹
    const Trajectory& getfinalTrajectory() const;
    
    // 获取相对轨迹
    const Trajectory& getRelativeTrajectory() const;
    
    // 辅助函数：根据方向添加单元格
    void addCellBasedOnDirection(Trajectory& trajectory,const GridCell& cell, int direction, bool isSixDirection);
    
    // 辅助函数：检查移动是否会超出边界
    bool wouldExceedBounds(const GridCell& cell, int direction, bool isSixDirection);
}; 