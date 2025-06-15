#include "GameObject.h"
#include <cmath>
#include <ctime>  // 添加ctime头文件用于time函数
#include <cstdlib> // 添加cstdlib头文件用于rand函数
#include <algorithm> // 添加algorithm头文件用于std::min函数
#include <vector>
#include<iostream>

using namespace std;

// 坐标范围常量
const int MIN_TRAJ_COORD = -15;
const int MAX_TRAJ_COORD = 15;

const vector<GridCell>hex_point={
    GridCell(0, -2),
    GridCell(0, 2),
    GridCell(-1, 1),
    GridCell(-1, -1),
    GridCell(1, 1),
    GridCell(1, -1),
    GridCell(-1,0),
    GridCell(1,0)
};
const vector<GridCell>four_directions={
    GridCell(-1, 0),
    GridCell(0, 1),
    GridCell(0, -1),
    GridCell(1, 0)
    
};
const vector<GridCell>hex_directions={
    GridCell(-2,0),
    GridCell(-1,-3),
    GridCell(1, -3),
    GridCell(2, 0),
    GridCell(1, 3),
    GridCell(-1, 3)
};
const int UP = 0;
const int RIGHT = 1;
const int LEFT = 2;
const int DOWN = 3;
const int UP_RIGHT = 5;
const int DOWN_RIGHT = 4;
const int DOWN_LEFT = 2;
const int UP_LEFT = 1;


GameObject::GameObject(int startRow, int startCol, const std::string& objectColor) {
    // 初始化游戏对象，设置起始位置和颜色
    // 将起始位置添加到实际轨迹中
    GridCell initialCell(startRow, startCol);
    actualTrajectory.addCell(initialCell);
}

const GridCell& GameObject::getCurrentCell(Trajectory&trajectory) const {
    // 返回当前位置
    return trajectory.getCurrentCell();
}
    

void GameObject::addCellBasedOnDirection(Trajectory& trajectory,const GridCell& cell, int direction, bool isSixDirection) {
    GridCell x;
    
    if (isSixDirection) {
        switch (direction) {
            case UP_RIGHT:
                x = cell+hex_directions[UP_RIGHT];
                break;
            case DOWN_RIGHT:
                x = cell+hex_directions[DOWN_RIGHT];
                break;
            case DOWN:
                x = cell+hex_directions[DOWN];
                break;
            case DOWN_LEFT:
                x = cell+hex_directions[DOWN_LEFT];
                break;
            case UP_LEFT:
                x = cell+hex_directions[UP_LEFT];
                break;
            case UP:
                x = cell+hex_directions[UP];
                break;  
        }
    } else {
        switch (direction) {
            case RIGHT:
                x = cell+four_directions[RIGHT];
                break;
            case UP:
                x = cell+four_directions[UP];
                break;
            case LEFT:
                x = cell+four_directions[LEFT];
                break;
            case DOWN:
                x = cell+four_directions[DOWN];
                break;
        }
    }
    
    trajectory.addCell(x);
}

void GameObject::removeFourDirection(Trajectory& trajectory,const GridCell& cell, int direction) {
    addCellBasedOnDirection(trajectory,cell, direction, false);
}

void GameObject::removeSixDirection(Trajectory& trajectory,const GridCell& cell, int direction) {
    addCellBasedOnDirection(trajectory,cell, direction, true);
}

bool checkIfExist(const Trajectory&trajectory,GridCell newgrid,const int direction,bool ISCOMPLEX){
    int i=trajectory.getLength();
    int value=0;
    if(ISCOMPLEX){
        for(int j=0;j<i;j++){
        if(trajectory.getCell(j)==(newgrid+hex_directions[direction])) value=1;}
    }
    else{
    for(int j=0;j<i;j++){
        if(trajectory.getCell(j)==(newgrid+four_directions[direction])) value=1;
    }
    }
    return value;
}

void GameObject::generateTrajectory(bool difficulty, int steps) {
    // 添加随机种子刷新，确保每次调用都获得新的随机序列
    srand(static_cast<unsigned int>(time(nullptr) * rand()));
    
        // 清空现有轨迹
        actualTrajectory.clear();

        // 生成随机初始坐标（范围-15到15）
        int startRow = (rand() % (MAX_TRAJ_COORD - MIN_TRAJ_COORD + 1)) + MIN_TRAJ_COORD;
        int startCol = (rand() % (MAX_TRAJ_COORD - MIN_TRAJ_COORD + 1)) + MIN_TRAJ_COORD;
        GridCell startCell(startRow, startCol);
        actualTrajectory.addCell(startCell);

    // 使用回溯算法生成轨迹
    int maxAttempts = 10;  // 最大尝试次数
    bool success = false;
    
    for (int attempt = 0; attempt < maxAttempts && !success; attempt++) {
        // 每次尝试使用新的随机种子
        srand(static_cast<unsigned int>(time(nullptr) * rand() + attempt));
        success = generateTrajectoryBacktrack(actualTrajectory, 0, steps, -1, difficulty);
        
        if (!success && attempt < maxAttempts - 1) {
            // 如果失败且还有尝试机会，清空轨迹并重新添加起始点
            actualTrajectory.clear();
            actualTrajectory.addCell(startCell);
        }
    }
    
    // 如果多次尝试后仍然失败，生成一个简单的线性轨迹
    /*if (!success) {
        cout << "使用备用轨迹生成方法" << endl;
        actualTrajectory.clear();
        actualTrajectory.addCell(startCell);
        
        // 简单的直线轨迹生成
        for (int i = 0; i < steps; i++) {
            const GridCell& currentCell = actualTrajectory.getCurrentCell();
            int row = currentCell.getRow();
            int col = currentCell.getCol();
            
            // 交替水平和垂直移动
            if (i % 2 == 0) {
                // 确保在边界内
                if (row + 1 <= MAX_TRAJ_COORD) {
                    GridCell newCell(row + 1, col);
                    actualTrajectory.addCell(newCell);
                } else {
                    GridCell newCell(row - 1, col);
                    actualTrajectory.addCell(newCell);
                }
            } else {
                // 确保在边界内
                if (col + 1 <= MAX_TRAJ_COORD) {
                    GridCell newCell(row, col + 1);
                    actualTrajectory.addCell(newCell);
                } else {
                    GridCell newCell(row, col - 1);
                    actualTrajectory.addCell(newCell);
                }
        }
    }
    }*/
}

void GameObject::generateRelativeTrajectory(int steps, bool difficulty) {
    // 添加随机种子刷新，确保每次调用都获得新的随机序列
    srand(static_cast<unsigned int>(time(nullptr) * rand() + 12345));
    
    // 清空现有相对轨迹
    relativeTrajectory.clear();
    
    // 生成随机初始坐标（范围-15到15）
    int startRow = (rand() % (MAX_TRAJ_COORD - MIN_TRAJ_COORD + 1)) + MIN_TRAJ_COORD;
    int startCol = (rand() % (MAX_TRAJ_COORD - MIN_TRAJ_COORD + 1)) + MIN_TRAJ_COORD;
    GridCell startCell(startRow, startCol);
    relativeTrajectory.addCell(startCell);
    
    // 使用回溯算法生成轨迹
    int maxAttempts = 10;  // 最大尝试次数
    bool success = false;
    
    for (int attempt = 0; attempt < maxAttempts && !success; attempt++) {
        // 每次尝试使用新的随机种子
        srand(static_cast<unsigned int>(time(nullptr) * rand() + 54321 + attempt));
        success = generateTrajectoryBacktrack(relativeTrajectory, 0, steps, -1, difficulty);
        
        if (!success && attempt < maxAttempts - 1) {
            // 如果失败且还有尝试机会，清空轨迹并重新添加起始点
            relativeTrajectory.clear();
            relativeTrajectory.addCell(startCell);
            }
    }
    
    // 如果多次尝试后仍然失败，生成一个简单的线性轨迹
    /*if (!success) {
        cout << "使用备用轨迹生成方法" << endl;
        relativeTrajectory.clear();
        relativeTrajectory.addCell(startCell);
        
        // 简单的直线轨迹生成
        for (int i = 0; i < steps; i++) {
            const GridCell& currentCell = relativeTrajectory.getCurrentCell();
            int row = currentCell.getRow();
            int col = currentCell.getCol();
            
            // 交替水平和垂直移动，与generateTrajectory相反方向
            if (i % 2 == 1) {
                // 确保在边界内
                if (row + 1 <= MAX_TRAJ_COORD) {
                    GridCell newCell(row + 1, col);
                    relativeTrajectory.addCell(newCell);
                } else {
                    GridCell newCell(row - 1, col);
                    relativeTrajectory.addCell(newCell);
                }
            } else {
                // 确保在边界内
                if (col + 1 <= MAX_TRAJ_COORD) {
                    GridCell newCell(row, col + 1);
                    relativeTrajectory.addCell(newCell);
                } else {
                    GridCell newCell(row, col - 1);
                    relativeTrajectory.addCell(newCell);
                }
        }
    }
    }
    */
}
    

void GameObject::calculateActualTrajectory() {
    // 添加随机种子刷新，确保每次调用都获得新的随机起点
    srand(static_cast<unsigned int>(time(nullptr) * rand() + 67890));
    
    // 清空现有实际轨迹
    finalTrajectory.clear();
    
    // 先获取两个轨迹的长度
    int actLength = actualTrajectory.getLength();
    int relLength = relativeTrajectory.getLength();
    
    // 确保至少有起始点
    if (actLength < 1 || relLength < 1) {
        return;
    }
    
    // 随机生成实际轨迹的起始点（范围-15到15）
double startRow = (rand()% (MAX_TRAJ_COORD - MIN_TRAJ_COORD + 1)) + MIN_TRAJ_COORD;
double startCol = (rand()% (MAX_TRAJ_COORD - MIN_TRAJ_COORD + 1)) + MIN_TRAJ_COORD;
finalTrajectory.addCell(GridCell(startRow, startCol));
    
    // 使用四方向移动生成实际轨迹，确保每次只移动1个单位
    int lastDirection = -1;
    int minLength = min(actLength, relLength);
    for(int i=1;i<minLength;i++){
        GridCell newCell = finalTrajectory.getCurrentCell()+
                         actualTrajectory.getCell(i)-actualTrajectory.getCell(i-1)+
                         relativeTrajectory.getCell(i)-relativeTrajectory.getCell(i-1);
        
        
        finalTrajectory.addCell(newCell);
    }
}


const Trajectory& GameObject::getfinalTrajectory() const {
    // 返回玩家预测的轨迹
    return finalTrajectory;
}

const Trajectory& GameObject::getRelativeTrajectory() const {
    // 返回相对轨迹
    return relativeTrajectory;
}

const Trajectory& GameObject::getActualTrajectory() const {
    // 返回相对轨迹
    return actualTrajectory;
}

bool GameObject::wouldExceedBounds(const GridCell& cell, int direction, bool isSixDirection) {
    GridCell newCell = cell;
    if (isSixDirection) {
        switch (direction) {
            case 0: newCell = cell + hex_directions[0]; break;
            case 1: newCell = cell + hex_directions[1]; break;
            case 2: newCell = cell + hex_directions[2]; break;
            case 3: newCell = cell + hex_directions[3]; break;
            case 4: newCell = cell + hex_directions[4]; break;
            case 5: newCell = cell + hex_directions[5]; break;
        }
    } else {
        switch (direction) {
            case 0: newCell = cell + four_directions[0]; break;
            case 1: newCell = cell + four_directions[1]; break;
            case 2: newCell = cell + four_directions[2]; break;
            case 3: newCell = cell + four_directions[3]; break;
        }
    }
    
    return (newCell.getRow() < MIN_TRAJ_COORD || newCell.getRow() > MAX_TRAJ_COORD ||
            newCell.getCol() < MIN_TRAJ_COORD || newCell.getCol() > MAX_TRAJ_COORD);
}

bool GameObject::generateTrajectoryBacktrack(Trajectory& trajectory, int depth, int maxDepth, int lastDir, bool isComplex) {
    // 达到目标深度，轨迹生成完成
    if (depth >= maxDepth) {
        return true;
    }
    
    // 获取当前单元格
    const GridCell& currentCell = trajectory.getCurrentCell();
    
    // 创建方向序列并随机打乱
    vector<int> directions;
    int numDirs = isComplex ? 6 : 4;
    for (int i = 0; i < numDirs; i++) {
        directions.push_back(i);
    }
    
    // 简单随机化方向顺序
    for (int i = 0; i < numDirs; i++) {
        int j = rand() % numDirs;
        swap(directions[i], directions[j]);
    }
    
    // 尝试每个方向
    for (int dir : directions) {
        // 如果这是相反方向，跳过（避免来回走）
        if (lastDir != -1) {
            if (isComplex && abs(dir - lastDir) == 3) continue;
            if (!isComplex && ((dir == 0 && lastDir == 3) ||
                               (dir == 3 && lastDir == 0) ||
                               (dir == 1 && lastDir == 2) ||
                               (dir == 2 && lastDir == 1))) continue;
        }
        
        // 如果新位置超出边界，跳过
        if (wouldExceedBounds(currentCell, dir, isComplex)) continue;
        
        // 如果新位置已经在轨迹中，跳过（避免环路）
        GridCell newCell;
        if (isComplex) {
            newCell = currentCell + hex_directions[dir];
        } else {
            newCell = currentCell + four_directions[dir];
        }
        bool cellExists = false;
        for (size_t i = 0; i < trajectory.getLength(); i++) {
            if (trajectory.getCell(i) == newCell) {
                cellExists = true;
                break;
            }
        }
        if (cellExists) continue;
        
        // 添加新单元格
        if (isComplex) {
            removeSixDirection(trajectory, currentCell, dir);
        } else {
            removeFourDirection(trajectory, currentCell, dir);
        }
        
        // 递归生成下一步
        if (generateTrajectoryBacktrack(trajectory, depth + 1, maxDepth, dir, isComplex)) {
            return true;
        }
        
        // 回溯：移除最后添加的单元格
        trajectory.getCells().pop_back();
    }
    
    // 所有方向都尝试过但没有解决方案
    return false;
}

