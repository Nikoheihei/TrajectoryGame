#pragma once
#include <vector>
#include "GridCell.h"

class Trajectory {
private:
    std::vector<GridCell> cells;  // 存储轨迹中的所有网格单元
    GridCell currentCell;        // 当前位置

public:
    // 构造函数
    Trajectory();
    
    // 添加一个网格单元到轨迹
    void addCell(const GridCell& cell);
    
    // 获取轨迹中所有网格单元
    const std::vector<GridCell>& getCells() const;
    
    // 获取轨迹中所有网格单元（非const版本，允许修改）
    std::vector<GridCell>& getCells();
    
    // 获取轨迹中网格单元的数量
    size_t getLength() const;
    
    // 获取指定索引处的网格单元
    GridCell getCell(size_t index) const;
    
    // 获取当前位置
    const GridCell& getCurrentCell() const;
    
    // 设置当前位置
    void setCurrentCell(const GridCell& cell);
    
    // 计算与另一条轨迹的相似度
    double calculateSimilarity(const Trajectory& other) const;
    
    // 清空轨迹
    void clear();
}; 