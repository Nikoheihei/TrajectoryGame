#pragma once

class GridCell {
private:
    int row;
    int col;//double因为每个位置图像可能是一个正六边形

public:
    // 构造函数
    GridCell(int r = 0, int c = 0);

    
    // 获取行列坐标
    int getRow() const;
    int getCol() const;
    
    // 比较两个网格单元是否相等
    bool operator==(const GridCell& other) const;
    bool operator!=(const GridCell& other) const;

    GridCell operator+(const GridCell& other)const;
    GridCell operator-(const GridCell& other)const;
}; 