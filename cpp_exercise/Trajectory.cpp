#include "Trajectory.h"
#include<algorithm>
#include <stdexcept>
using namespace std;
//Trajectory内存储着一堆GridCell，表示一个对象的移动轨迹
Trajectory::Trajectory() {
    // 构造函数初始化空轨迹
    // currentCell默认为(0,0)
}

void Trajectory::addCell(const GridCell& cell) {
    // 向轨迹中添加一个网格单元
    cells.push_back(cell);
    // 同时更新当前位置
    setCurrentCell(cell);
}

const std::vector<GridCell>& Trajectory::getCells() const {
    // 返回包含所有网格单元的向量
    return cells;
}

std::vector<GridCell>& Trajectory::getCells() {
    // 返回包含所有网格单元的向量（非const版本）
    return cells;
}

size_t Trajectory::getLength() const {
    // 返回轨迹中网格单元的数量
    return cells.size();
}

GridCell Trajectory::getCell(size_t index) const {
    // 返回指定索引处的网格单元
     //边界检查    
    if(index >= cells.size()|| index < 0) { 
        throw std::out_of_range("Index out of range"); }// 抛出异常以处理越界访问
    return cells[index];
}

const GridCell& Trajectory::getCurrentCell() const {
    // 返回当前位置
    return currentCell;
}

void Trajectory::setCurrentCell(const GridCell& cell) {
    // 设置当前位置
    currentCell = cell;
}

double Trajectory::calculateSimilarity(const Trajectory& other) const {
     // 计算两条轨迹的相似度
    // 例如，从头开始连续重合的单元格比例
    // 返回值范围：0.0（完全不同）到1.0（完全相同）用于评分
    double c;
    int size=min(cells.size(),other.cells.size())-1;
    if(size){
    double a=0;
    for(int i=1;i<=size;i++){
        if(cells[i]==other.cells[i]){
            a+=1;
        }
    }
    c= a/size;}
    else{
        printf("轨迹出错了！");
    }
    return c;
}

void Trajectory::clear() {
    // 清空轨迹
    cells.clear();
    // 重置当前位置为默认值
    //currentCell = GridCell(0, 0);
} 