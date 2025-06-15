#include "GameManager.h"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <chrono>
#include <thread>
#include <map>
#include <algorithm>
#include <limits>
#if defined(_WIN32)
#include <windows.h>
#endif
using namespace std;

// 函数声明
void runMultiplayerGame(GameManager &gameManager);
void displayTrajectories(const GameObject &objectA, const Trajectory &predictedPath, bool isComplexMode, bool showFinalTrajectory);
Trajectory inputPrediction(const GameObject &objectA, int steps, bool isComplexMode);
void savePlayerScore(const string &username, const string &mode, int score);
void runSinglePlayerGame(GameManager &gameManager);
void BeginGame(GameManager &gameManager, string username);
void showPlayerStats(const string &username);
void showGameRankings();

// 自定义快速排序算法的分区函数
template <typename T, typename Compare>
int partition(std::vector<T> &arr, int low, int high, Compare comp)
{
    // 选择最右边的元素作为基准
    T pivot = arr[high];
    // 较小元素的索引
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++)
    {
        // 如果当前元素小于或等于基准
        if (comp(arr[j], pivot) || !comp(pivot, arr[j]))
        {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return (i + 1);
}

// 实现快速排序算法
template <typename T, typename Compare>
void quickSort(std::vector<T> &arr, int low, int high, Compare comp)
{
    if (low < high)
    {
        // 获取分区索引
        int pi = partition(arr, low, high, comp);

        // 分别对分区左右两部分进行排序
        quickSort(arr, low, pi - 1, comp);
        quickSort(arr, pi + 1, high, comp);
    }
}

// 对外暴露的排序接口，简化调用
template <typename T, typename Compare>
void customQuickSort(std::vector<T> &arr, Compare comp)
{
    if (arr.size() > 1)
    {
        quickSort(arr, 0, arr.size() - 1, comp);
    }
}

// 用于控制台可视化的网格大小
const int GRID_SIZE = 61;
const char EMPTY_CELL = '.';
const char ACTUAL_PATH = 'A';
const char RELATIVE_PATH = 'R';
const char PREDICTED_PATH = 'P';
const char OVERLAP_AR = 'C';  // 实际和相对轨迹重叠
const char OVERLAP_AP = 'M';  // 实际和预测轨迹重叠
const char OVERLAP_RP = 'O';  // 相对和预测轨迹重叠
const char OVERLAP_ALL = '*'; // 所有轨迹重叠
const int MIN_GRID_COORD = -30;
const int MAX_GRID_COORD = 30;
const int MIN_TRAJ_COORD = -15;
const int MAX_TRAJ_COORD = 15;

const vector<GridCell> hex_point = {
    GridCell(0, -2),
    GridCell(0, 2),
    GridCell(-1, 1),
    GridCell(-1, -1),
    GridCell(1, 1),
    GridCell(1, -1),
    GridCell(-1, 0),
    GridCell(1, 0)};
const vector<GridCell> four_directions = {
    GridCell(-1, 0),
    GridCell(0, 1),
    GridCell(0, -1),
    GridCell(1, 0)

};
const vector<GridCell> hex_directions = {
    GridCell(-2, 0),
    GridCell(-1, -3),
    GridCell(1, -3),
    GridCell(2, 0),
    GridCell(1, 3),
    GridCell(-1, 3)};
const int UP = 0;
const int RIGHT = 1;
const int LEFT = 2;
const int DOWN = 3;
const int UP_RIGHT = 5;
const int DOWN_RIGHT = 4;
const int DOWN_LEFT = 2;
const int UP_LEFT = 1;

const string userInfoFile = "userInfor.txt";
const string doublePlayerResultFile = "doublePlayerResult.txt";

// 用于显示轨迹的函数
void displayTrajectories(const GameObject &objectA, const Trajectory &predictedPath, bool isComplexMode, bool showFinalTrajectory)
{
    // 创建一个空的网格
    vector<vector<string>> grid(GRID_SIZE, vector<string>(GRID_SIZE, "."));

    // 获取轨迹
    const Trajectory &actualTrajectory = objectA.getActualTrajectory();
    const Trajectory &relativeTrajectory = objectA.getRelativeTrajectory();
    const Trajectory &finalTrajectory = objectA.getfinalTrajectory();

    // 确定网格范围 -30到30
    const int MIN_COORD = MIN_GRID_COORD;
    const int MAX_COORD = MAX_GRID_COORD;
    const int OFFSET = MAX_COORD; // 用于将坐标转换为索引的偏移量

    // 实际上可视化是靠一个二维vector存储得来的

    // 填充网格 - 实际轨迹 (使用 A0, A1, A2, ...)
    if (!showFinalTrajectory)
    {
        for (size_t i = 0; i < actualTrajectory.getLength(); i++)
        {
            const GridCell &cell = actualTrajectory.getCell(i);
            int row = cell.getRow() + OFFSET;
            int col = cell.getCol() + OFFSET;

            // 确保在网格范围内
            if (!isComplexMode)
            {
                if (row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE)
                {
                    string marker = "A" + to_string(i % 10); // 使用数字标记顺序
                    grid[row][col] = marker;
                }
            }
            else
            {
                if (row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE)
                {
                    string marker = "A" + to_string(i % 10); // 使用数字标记顺序
                    for (int j = 0; j < 8; j++)
                    {
                        int gridX = row + hex_point[j].getRow();
                        int gridY = col + hex_point[j].getCol();
                        if (gridX >= 0 && gridX < GRID_SIZE && gridY >= 0 && gridY < GRID_SIZE && grid[gridX][gridY] == ".")
                        {
                            grid[gridX][gridY] = "#";
                        }
                    }
                    grid[row][col] = marker;
                }
            }
        }
    }

    // 填充网格 - 相对轨迹 (使用 R0, R1, R2, ...)
    if (!showFinalTrajectory)
    {
        for (size_t i = 0; i < relativeTrajectory.getLength(); i++)
        {
            const GridCell &cell = relativeTrajectory.getCell(i);
            int row = cell.getRow() + OFFSET;
            int col = cell.getCol() + OFFSET;

            // 确保在网格范围内
            if (row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE)
            {
                string marker = "R" + to_string(i % 10);
                // 如果已经有标记，表示重叠
                if (grid[row][col] != "." && grid[row][col] != "+")
                {
                    if (grid[row][col][0] == 'A')
                    {
                        marker = "C" + to_string(i % 10); // A和R重叠
                    }
                    else if (grid[row][col][0] == 'P')
                    {
                        marker = "O" + to_string(i % 10); // R和P重叠
                    }
                    else if (grid[row][col][0] == 'C')
                    {
                        marker = "*" + to_string(i % 10); // 全部重叠
                    }
                }

                if (isComplexMode)
                {
                    for (int j = 0; j < 8; j++)
                    {
                        int gridX = row + hex_point[j].getRow();
                        int gridY = col + hex_point[j].getCol();
                        if (gridX >= 0 && gridX < GRID_SIZE && gridY >= 0 && gridY < GRID_SIZE && grid[gridX][gridY] == ".")
                        {
                            grid[gridX][gridY] = "&";
                        }
                    }
                }
                grid[row][col] = marker;
            }
        }
    }

    // 填充网格 - 预测轨迹 (使用 P0, P1, P2, ...)
    for (size_t i = 0; i < predictedPath.getLength(); i++)
    {
        const GridCell &cell = predictedPath.getCell(i);
        // 添加安全检查，确保i不超过finalTrajectory的长度
        bool canCompareWithWishCell = (i < finalTrajectory.getLength());
        int row = cell.getRow() + OFFSET;
        int col = cell.getCol() + OFFSET;

        // 确保在网格范围内
        if (row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE)
        {
            string marker = "P" + to_string(i % 10);
            // 处理重叠情况
            if (grid[row][col] != ".")
            {
                if (grid[row][col][0] == 'A')
                {
                    marker = "M" + to_string(i % 10); // A和P重叠
                }
                else if (grid[row][col][0] == 'R')
                {
                    marker = "O" + to_string(i % 10); // R和P重叠
                }
                else if (grid[row][col][0] == 'C')
                {
                    marker = "*" + to_string(i % 10); // 全部重叠
                }
            }

            if (isComplexMode)
            {
                for (int j = 0; j < 8; j++)
                {
                    int gridX = row + hex_point[j].getRow();
                    int gridY = col + hex_point[j].getCol();
                    if (gridX >= 0 && gridX < GRID_SIZE && gridY >= 0 && gridY < GRID_SIZE && grid[gridX][gridY] == ".")
                    {
                        grid[gridX][gridY] = "&";
                    }
                }
            }
            grid[row][col] = marker;
        }
    }

    if (showFinalTrajectory)
    {
        cout << endl;
    }
    else
    {
        std::cout << "A0-A9 - 参考轨迹，R0-R9 - 相对轨迹，P0-P9 - 预测轨迹" << endl;
        std::cout << "C - 参考和相对重叠，M - 参考和预测重叠，O - 相对和预测重叠，* - 全部重叠" << endl;
    }

    std::cout << "数字表示轨迹中点的顺序 (0-9循环)" << endl;

    // 打印坐标轴标签
    std::cout << "\n坐标范围：X轴和Y轴从 " << MIN_COORD << " 到 " << MAX_COORD << endl;

    // 打印网格内容 - 修正行标签对齐
    for (int i = 0; i < GRID_SIZE; i++)
    {
        if (i % 5 == 0)
        {
            int originalCoord = MIN_GRID_COORD + (i);
            cout << setw(5) << originalCoord << " "; // 统一使用5个字符宽度
        }
        else
        {
            cout << "      "; // 保持6个字符的空间
        }

        for (int j = 0; j < GRID_SIZE; j++)
        {
            cout << setw(2) << grid[i][j] << " ";
        }
        cout << endl;
    }

    // 显示系统生成的完整实际轨迹（用于测试）
    cout << "\n系统生成的实际轨迹（通过计算得到）：" << endl;
    for (size_t i = 0; i < finalTrajectory.getLength(); i++)
    {
        const GridCell &cell = finalTrajectory.getCell(i);
        cout << "  点" << i << ": 原始坐标("
             << cell.getRow() << "," << cell.getCol() << ")" << endl;
    }
}

// 手动输入预测轨迹
Trajectory inputPrediction(const GameObject &objectA, int steps, bool isComplexMode)
{
    Trajectory prediction;
    const Trajectory &finalTrajectory = objectA.getfinalTrajectory();
    int finalLength = finalTrajectory.getLength();

    std::cout << "请输入预测轨迹（" << steps << "步）" << std::endl;

    // 预测轨迹的起始点要求和电脑通过actualTrajectory和
    // RelativeTrajectory计算得出的finalTrajectory的起始点一致。
    prediction.addCell(finalTrajectory.getCell(0));
    cout << "起始点行坐标（相对于中心0）：" << prediction.getCurrentCell().getRow() << endl;
    cout << "起始点列坐标（相对于中心0）：" << prediction.getCurrentCell().getCol() << endl;

    // 输入每一步的方向
    for (int i = 0; i < steps; i++)
    {
        int x, y;
        // 显示当前的轨迹
        displayTrajectories(objectA, prediction, isComplexMode, false);

        // 显示已输入的所有预测坐标
        if (prediction.getLength() > 1)
        { // 已经有输入的坐标
            cout << "\n已输入的预测坐标:" << endl;
            for (size_t j = 1; j < prediction.getLength(); j++)
            { // 从索引1开始，跳过起始点
                const GridCell &cell = prediction.getCell(j);
                cout << "  步骤 " << j << ": (" << cell.getRow() << ", " << cell.getCol() << ")" << endl;
            }
        }

        cout << "\n当前输入第 " << (i + 1) << " 步的坐标" << endl;
        cout << "请输入下一个位置的行坐标（-30到30之间）：";
        cin >> x;
        cout << "请输入下一个位置的列坐标（-30到30之间）：";
        cin >> y;

        // 添加输入验证
        while (x < -30 || x > 30 || y < -30 || y > 30)
        {
            cout << "\n坐标超出范围！请重新输入（-30到30之间）" << endl;
            cout << "行坐标：";
            cin >> x;
            cout << "列坐标：";
            cin >> y;
        }

        GridCell newCell(x, y);
        prediction.addCell(newCell);
    }
    displayTrajectories(objectA, prediction, isComplexMode, false);

    // 显示所有预测坐标
    cout << "\n完整的预测轨迹坐标:" << endl;
    for (size_t i = 0; i < prediction.getLength(); i++)
    {
        const GridCell &cell = prediction.getCell(i);
        cout << "  点 " << i << ": (" << cell.getRow() << ", " << cell.getCol() << ")" << endl;
    }

    return prediction;
}

// 保存玩家得分到文件
void savePlayerScore(const string &username, const string &mode, int score)
{
    try
    {
        // 先检查文件是否存在
        bool fileExists = false;
        ifstream checkFile("scores.txt");
        fileExists = checkFile.good();
        checkFile.close();

        ofstream scoreFile;
        if (!fileExists)
        {
            // 如果文件不存在，创建新文件并添加标题
            scoreFile.open("scores.txt", ios::out);
            if (scoreFile.is_open())
            {
                scoreFile << "username,mode,score" << endl;
            }
        }
        else
        {
            // 如果文件存在，追加模式打开
            scoreFile.open("scores.txt", ios::app);
        }

        if (scoreFile.is_open())
        {
            // 写入分数记录
            scoreFile << username << " " << mode << " " << score << endl;
            scoreFile.close();
        }
        else
        {
            cout << "无法打开分数文件进行写入!" << endl;
        }
    }
    catch (const exception &e)
    {
        cout << "保存分数时发生错误: " << e.what() << endl;
    }
}

// 运行单人游戏
void runSinglePlayerGame(GameManager &gameManager)
{
    bool isComplexMode = gameManager.isComplexMode();
    bool isTimeBasedMode = gameManager.isTimeBasedGame();
    // 获取游戏对象
    const GameObject &objectA = gameManager.getObjectA();

    // 显示初始轨迹
    Trajectory emptyPrediction; // 创建一个空的预测轨迹用于初始显示
    cout << "\n初始轨迹：" << endl;
    cout << "A - 参考轨迹(蓝色物体的运动)" << endl;
    cout << "R - 相对轨迹(红色物体相对于蓝色物体的运动)" << endl;
    cout << "\n请预测红色物体在实际坐标系中的运动轨迹" << endl;

    int predictionSteps = 10;
    Trajectory userPrediction = inputPrediction(objectA, predictionSteps, isComplexMode);

    // 输出评分
    double similarity = userPrediction.calculateSimilarity(objectA.getfinalTrajectory());
    cout << "相似度: " << similarity * 100 << "%" << endl;
    int score = similarity * 1000;
    gameManager.getCurrentPlayer().addScore(score);
    cout << "得分: " << score << endl;

    // 保存分数到文件，但在try-catch块中处理
    try
    {
        string mode = isComplexMode ? "COMPLEX_SINGLE" : "SIMPLE_SINGLE";
        if (isTimeBasedMode)
        {
            mode = "TIME_BASED_MODE";
        }
        else if (gameManager.isMultiplayerMode())
        {
            mode = isComplexMode ? "COMPLEX_MULTI" : "SIMPLE_MULTI";
        }

        // 确定保存用户名 - 优先使用登录用户名，如果当前玩家名称为空
        string playerName = gameManager.getCurrentPlayer().getName();
        if (playerName.empty() && gameManager.isUserLoggedIn())
        {
            playerName = gameManager.getLoggedInUsername();
            cout << "使用登录用户名: " << playerName << " 保存分数" << endl;
        }
        else if (playerName.empty())
        {
            cout << "警告: 玩家名称为空，将使用'未知玩家'作为替代" << endl;
            playerName = "未知玩家";
        }

        // 尝试方式1：使用C风格文件I/O (最安全的方式)
        bool saved = false;

        try
        {
            FILE *fp = fopen("scores.txt", "a");
            if (fp)
            {
                fprintf(fp, "%s %s %d\n", playerName.c_str(), mode.c_str(), score);
                fclose(fp);
                saved = true;
                cout << "分数已成功保存" << endl;
            }
            else
            {
                perror("fopen error");
            }
        }
        catch (...)
        {
            cout << "保存方式1失败，尝试备选方式..." << endl;
        }

        // 如果方式1失败，尝试方式2
        if (!saved)
        {
            try
            {
                string currentDir = "./scores.txt";
                ofstream outFile(currentDir, ios::app);
                if (outFile.is_open())
                {
                    outFile << playerName << " " << mode << " " << score << endl;
                    outFile.close();
                    saved = true;
                    cout << "成功保存到当前目录: " << currentDir << endl;
                }
            }
            catch (...)
            {
                cout << "保存方式2失败，尝试最后一种方式..." << endl;
            }
        }

        // 最后的备用方式
        if (!saved)
        {
            try
            {
                string tempFile = "temp_scores.txt";
                ofstream tempOut(tempFile);
                if (tempOut.is_open())
                {
                    tempOut << playerName << " " << mode << " " << score << endl;
                    tempOut.close();
                    cout << "成功保存到临时文件: " << tempFile << endl;
                    saved = true;
                }
            }
            catch (...)
            {
                cout << "所有保存方式均失败，但游戏将继续。" << endl;
            }
        }
    }
    catch (const std::bad_alloc &e)
    {
        cout << "内存分配错误: " << e.what() << " (游戏将继续)" << endl;
    }
    catch (const exception &e)
    {
        cout << "保存分数时发生错误: " << e.what() << " (游戏将继续)" << endl;
    }
    catch (...)
    {
        cout << "保存分数时发生未知错误，但游戏将继续。" << endl;
    }

    cout << "\n=== 游戏结束 ===" << endl;
    cout << "最终得分: " << score << endl;

    // 暂停程序等待用户交互
    cout << "\n按任意键继续..." << endl;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 清除输入缓冲区
    cin.get();                                           // 等待用户按键

// 清屏
#if defined(_WIN32)
    system("cls");
#else
    system("clear");
#endif
}

void BeginGame(GameManager &gameManager, string username)
{
    int gameMode;
    string username2;
    bool validPlayer2 = false;

    cout << "\n=== 选择游戏模式 ===" << endl;
    cout << "1. 简单单人模式" << endl;
    cout << "2. 复杂单人模式" << endl;
    cout << "3. 简单多人模式" << endl;
    cout << "4. 复杂多人模式" << endl;
    cout << "5. 计时多人模式" << endl;
    cout << "6. 返回主菜单" << endl;
    cout << "请选择: ";
    cin >> gameMode;

    // 初始化游戏
    switch (gameMode)
    {
    case 1:
        // 确保使用登录的用户名
        if (gameManager.isUserLoggedIn() && username.empty())
        {
            username = gameManager.getLoggedInUsername();
        }

        gameManager.initializeGame(GameManager::SIMPLE_SINGLE, username, "");
        cout << "\n=== " << "简单" << "单人游戏开始 ===" << endl;
        cout << "当前玩家: " << username << endl;
        cout << "游戏原理：电脑生成一个轨迹A和一个轨迹B相对A的相对路径" << endl;
        cout << "玩家任务是预测轨迹B在实际坐标系中的运动路径\n"
             << endl;
        runSinglePlayerGame(gameManager);
        break;
    case 2:
        // 确保使用登录的用户名
        if (gameManager.isUserLoggedIn() && username.empty())
        {
            username = gameManager.getLoggedInUsername();
        }

        gameManager.initializeGame(GameManager::COMPLEX_SINGLE, username, "");
        cout << "\n=== " << "复杂" << "单人游戏开始 ===" << endl;
        cout << "当前玩家: " << username << endl;
        cout << "游戏原理：电脑生成一个轨迹A和一个轨迹B相对A的相对路径" << endl;
        cout << "玩家任务是预测轨迹B在实际坐标系中的运动路径\n"
             << endl;

        runSinglePlayerGame(gameManager);
        break;
    case 3:
    case 4:
    case 5:
        // 多人模式验证玩家2是否存在
        while (!validPlayer2)
        {
            cout << "请输入玩家2的名称：";
            cin >> username2;

            // 检查玩家2是否存在
            if (username2 == username)
            {
                cout << "不能与自己对战" << endl;
                continue;
            }
            fstream userFile;
            userFile.open(userInfoFile, ios::in);
            if (gameManager.CheakIfUserExist(userFile, username2))
            {
                validPlayer2 = true;
                userFile.close();
            }
            else
            {
                cout << "该用户不存在! 请先注册该用户或重新输入合法用户名" << endl;
                cout << "1. 重新输入用户名" << endl;
                cout << "2. 返回主菜单" << endl;
                int choice;
                cin >> choice;
                userFile.close();

                if (choice == 2)
                {
                    return; // 返回主菜单
                }
                // 否则继续循环要求输入合法用户名
            }
        }

        // 根据选择初始化对应的多人游戏模式
        if (gameMode == 3)
        {
            gameManager.initializeGame(GameManager::SIMPLE_MULTI, username, username2);
            runMultiplayerGame(gameManager);
        }
        else if (gameMode == 4)
        {
            gameManager.initializeGame(GameManager::COMPLEX_MULTI, username, username2);
            runMultiplayerGame(gameManager);
        }
        else
        { // gameMode == 5
            gameManager.initializeGame(GameManager::TIME_BASED_MODE, username, username2);
            runMultiplayerGame(gameManager);
        }
        break;
    case 6:
        // 直接返回主菜单
        return;
    default:
        cout << "无效选择!" << endl;
        break;
    }

    // 游戏结束后，询问用户是否再玩一次
    cout << "\n是否要再玩一次游戏?" << endl;
    cout << "1. 再玩一次" << endl;
    cout << "2. 返回主菜单" << endl;
    cout << "请选择: ";
    int playAgain;
    cin >> playAgain;

    if (playAgain == 1)
    {
        BeginGame(gameManager, username); // 递归调用自己，开始新游戏
    }
    else
    {
// 清屏后返回主菜单
#if defined(_WIN32)
        system("cls");
#else
        system("clear");
#endif
    }
    // 函数结束返回主菜单
}

int main()
{
    srand(static_cast<unsigned int>(time(0)));
#if defined(_WIN32)
    // 切换控制台到 UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // 启动时清屏
    system("cls");
#else
    // 启动时清屏
    system("clear");
#endif
    // 创建游戏管理器实例
    GameManager gameManager;

    int choice = 0;
    string username;
    fstream userFile;

    cout << "欢迎来到轨迹预测游戏!" << endl;

    while (true)
    {
        if (!gameManager.isUserLoggedIn())
        {
            // 未登录状态下的菜单
            cout << "\n=== 主菜单 ===" << endl;
            cout << "1. 注册新用户" << endl;
            cout << "2. 用户登录" << endl;
            cout << "3. 查询个人统计" << endl;
            cout << "4. 查询游戏排名" << endl;
            cout << "5. 退出游戏" << endl;
            cout << "请选择: ";
            cin >> choice;

            switch (choice)
            {
            case 1: // 注册新用户
                cout << "请输入用户名: ";
                cin >> username;
                userFile.open(userInfoFile, ios::in);
                if (!gameManager.CheakIfUserExist(userFile, username))
                {
                    userFile.close();
                    userFile.clear(); // 清除文件流状态标志
                    userFile.open(userInfoFile, ios::out | ios::app);

                    if (userFile.is_open()) {
                        gameManager.registerUser(userFile, username);
                        cout << "注册成功!" << endl;
                    } else {
                        cout << "无法打开文件进行写入!" << endl;
                    }
                }
                else
                {
                    cout << "用户名已存在!" << endl;
                }
                userFile.close();
                break;

            case 2: // 用户登录
                cout << "请输入用户名: ";
                cin >> username;
                userFile.open(userInfoFile, ios::in);
                if (gameManager.loginUser(userFile, username))
                {
                    cout << "登录成功!" << endl;
                    userFile.close();
                }
                else
                {
                    cout << "用户名不存在!请先注册" << endl;
                    userFile.close();
                }
                break;

            case 3: // 查询个人统计
                cout << "请输入用户名: ";
                cin >> username;
                userFile.open(userInfoFile, ios::in);
                if (gameManager.CheakIfUserExist(userFile, username))
                {
                    userFile.close();
                    showPlayerStats(username);
                }
                else
                {
                    cout << "用户名不存在!请先注册" << endl;
                    userFile.close();
                }
                break;

            case 4: // 查询游戏排名
                showGameRankings();
                break;

            case 5: // 退出
                cout << "谢谢使用，再见!" << endl;
                return 0;

            default:
                cout << "无效选择，请重试!" << endl;
            }
        }
        else
        {
            // 已登录状态下的菜单
            username = gameManager.getLoggedInUsername();
            cout << "\n=== 主菜单 (当前用户: " << username << ") ===" << endl;
            cout << "1. 开始游戏" << endl;
            cout << "2. 查询个人统计" << endl;
            cout << "3. 查询游戏排名" << endl;
            cout << "4. 退出登录" << endl;
            cout << "5. 退出游戏" << endl;
            cout << "请选择: ";
            cin >> choice;

            switch (choice)
            {
            case 1: // 开始游戏
                BeginGame(gameManager, username);
// 清屏
#if defined(_WIN32)
                system("cls");
#else
                system("clear");
#endif
                break;

            case 2: // 查询个人统计
                showPlayerStats(username);
                // 暂停程序等待用户交互
                cout << "\n按任意键继续..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 清除输入缓冲区
                cin.get();                                           // 等待用户按键
// 清屏
#if defined(_WIN32)
                system("cls");
#else
                system("clear");
#endif
                break;

            case 3: // 查询游戏排名
                showGameRankings();
                // 暂停程序等待用户交互
                cout << "\n按任意键继续..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 清除输入缓冲区
                cin.get();                                           // 等待用户按键
// 清屏
#if defined(_WIN32)
                system("cls");
#else
                system("clear");
#endif
                break;

            case 4: // 退出登录
                gameManager.logout();
                cout << "已退出登录!" << endl;
                break;

            case 5: // 退出游戏
                cout << "谢谢使用，再见!" << endl;
                return 0;

            default:
                cout << "无效选择，请重试!" << endl;
            }
        }
    }

    return 0;
}

// 运行多人游戏
void runMultiplayerGame(GameManager &gameManager)
{
    bool isComplexMode = gameManager.isComplexMode();
    bool isTimeBasedMode = gameManager.isTimeBasedGame();

    if (gameManager.getCurrentGameMode() == GameManager::TIME_BASED_MODE)
    {
        cout << "\n=== " << "计时" << "多人游戏开始 ===" << endl;
        gameManager.updateTotalRounds(2);
    }
    else
    {
        cout << "\n=== " << (isComplexMode ? "复杂" : "简单") << "多人游戏开始 ===" << endl;
    }

    // 设置初始玩家为第一个玩家
    gameManager.switchPlayer(); // 第一次调用会设置为玩家0

    // 计时模式需要确保每个玩家都进行游戏
    if (isTimeBasedMode)
    {
        cout << "当前玩家: " << gameManager.getCurrentPlayer().getName() << endl;

        // 生成游戏数据
        srand(static_cast<unsigned int>(time(nullptr) + gameManager.getCurrentRound()));
        gameManager.generateGameData();

        // 开始计时
        gameManager.getCurrentPlayer().startTimer();
        runSinglePlayerGame(gameManager);
        gameManager.getCurrentPlayer().endTimer();

        // 显示得分
        cout << "\n得分：" << endl;
        cout << "玩家" << gameManager.getCurrentPlayer().getName() << ": "
             << gameManager.getCurrentPlayer().getTotalScore() << endl;

        // 切换到第二个玩家
        gameManager.switchPlayer();
        std::this_thread::sleep_for(std::chrono::seconds(2));

        cout << "当前玩家: " << gameManager.getCurrentPlayer().getName() << endl;

        // 第二个玩家不需要重新生成数据，使用与第一个玩家相同的数据
        // 只需要调用generateGameData函数，它会自动共享第一个玩家的数据
        gameManager.generateGameData();

        // 开始计时
        gameManager.getCurrentPlayer().startTimer();
        runSinglePlayerGame(gameManager);
        gameManager.getCurrentPlayer().endTimer();

        // 显示得分
        cout << "\n得分：" << endl;
        for (const auto &player : gameManager.getPlayers())
        {
            cout << "玩家" << player.getName() << ": " << player.getTotalScore() << endl;
        }
    }
    else
    {
        // 普通多人模式
        cout << "游戏将进行 " << gameManager.getTotalRounds() + 1 << " 回合" << endl;
        while (!gameManager.isGameOver())
        {
            cout << "\n=== 第 " << gameManager.getCurrentRound() + 1 << " 轮 ===" << endl;
            cout << "当前玩家: " << gameManager.getCurrentPlayer().getName() << endl;

            // 生成游戏数据 - 现在会根据玩家顺序自动决定是生成新数据还是使用共享数据
            gameManager.generateGameData();
            runSinglePlayerGame(gameManager);

            // 显示当前回合所有玩家的得分
            cout << "\n得分：" << endl;
            for (const auto &player : gameManager.getPlayers())
            {
                cout << "玩家" << player.getName() << ": " << player.getTotalScore() << endl;
            }

            // 如果游戏还未结束，切换到下一个玩家
            if (!gameManager.isGameOver())
            {
                gameManager.switchPlayer();
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }
    }

    // 显示最终结果
    cout << "\n=== 游戏结束 ===" << endl;
    fstream doublePlayerFile;
    doublePlayerFile.open(doublePlayerResultFile, ios::out | ios::app);

    if (isTimeBasedMode)
    {
        // 计算总时间并决定获胜者
        double time1 = gameManager.getPlayer(0).getTimeTaken();
        double time2 = gameManager.getPlayer(1).getTimeTaken();

        cout << "总用时：" << endl;
        cout << "玩家" << gameManager.getPlayer(0).getName() << ": " << time1 << " 秒" << endl;
        cout << "玩家" << gameManager.getPlayer(1).getName() << ": " << time2 << " 秒" << endl;

        if (gameManager.getPlayer(0).getTotalScore() == gameManager.getPlayer(1).getTotalScore())
        {
            if (time1 < time2)
            {
                cout << "\n获胜者: 玩家" << gameManager.getPlayer(0).getName() << "!" << endl;
                gameManager.saveDoublePlayerResult(doublePlayerFile, gameManager.getPlayer(0).getName(),
                                                   gameManager.getPlayer(1).getName());
            }
            else if (time2 < time1)
            {
                cout << "\n获胜者: 玩家" << gameManager.getPlayer(1).getName() << "!" << endl;
                gameManager.saveDoublePlayerResult(doublePlayerFile, gameManager.getPlayer(1).getName(),
                                                   gameManager.getPlayer(0).getName());
            }
            else
            {
                cout << "\n平局!" << endl;
            }
        }
        else if (gameManager.getPlayer(0).getTotalScore() > gameManager.getPlayer(1).getTotalScore())
        {
            cout << "\n获胜者: 玩家" << gameManager.getPlayer(0).getName() << "!" << endl;
            gameManager.saveDoublePlayerResult(doublePlayerFile, gameManager.getPlayer(0).getName(),
                                               gameManager.getPlayer(1).getName());
        }
        else
        {
            cout << "\n获胜者: 玩家" << gameManager.getPlayer(1).getName() << "!" << endl;
            gameManager.saveDoublePlayerResult(doublePlayerFile, gameManager.getPlayer(1).getName(),
                                               gameManager.getPlayer(0).getName());
        }
    }
    else // 普通模式下如何计算成绩
    {
        // 显示最终得分
        cout << "最终得分：" << endl;
        int maxScore = 0;
        string winner;
        int index = -1;

        for (const auto &player : gameManager.getPlayers())
        {
            int score = player.getTotalScore();
            cout << "玩家" << player.getName() << ": " << score << endl;

            if (score > maxScore)
            {
                maxScore = score;
                winner = player.getName();
                index += 1;
            }
        }

        if (!winner.empty())
        {
            cout << "\n获胜者: " << winner << "!" << endl;
            gameManager.saveDoublePlayerResult(doublePlayerFile, winner,
                                               gameManager.getPlayer(abs(index - 1)).getName());
        }
        else
        {
            cout << "\n平局!" << endl;
        }
    }

    // 暂停程序等待用户交互
    cout << "\n按任意键继续..." << endl;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 清除输入缓冲区
    cin.get();                                           // 等待用户按键

// 清屏
#if defined(_WIN32)
    system("cls");
#else
    system("clear");
#endif
}

// 显示玩家统计信息
void showPlayerStats(const string &username)
{
    try
    {
        // 统计数据
        int simpleTotal = 0;    // 单人简单模式总次数
        int simplePerfect = 0;  // 单人简单模式满分次数
        int complexTotal = 0;   // 单人复杂模式总次数
        int complexPerfect = 0; // 单人复杂模式满分次数

        // 打开并读取scores.txt
        ifstream scoreFile("scores.txt");
        if (!scoreFile.is_open())
        {
            cout << "无法打开分数文件!" << endl;
            return;
        }

        string line;
        // 跳过标题行
        // getline(scoreFile, line);

        // 读取并统计数据
        string user, mode;
        int score;
        while (scoreFile >> user >> mode >> score)
        {
            if (user == username)
            {
                if (mode == "SIMPLE_SINGLE")
                {
                    simpleTotal++;
                    if (score == 1000)
                    {
                        simplePerfect++;
                    }
                }
                else if (mode == "COMPLEX_SINGLE")
                {
                    complexTotal++;
                    if (score == 1000)
                    {
                        complexPerfect++;
                    }
                }
            }
        }
        scoreFile.close();

        // 计算满分率
        double simpleRate = simpleTotal > 0 ? (double)simplePerfect / simpleTotal * 100 : 0;
        double complexRate = complexTotal > 0 ? (double)complexPerfect / complexTotal * 100 : 0;

        // 显示单人模式统计
        cout << "\n=== " << username << " 的个人统计 ===" << endl;
        cout << "简单单人模式:" << endl;
        cout << "  总游戏次数: " << simpleTotal << endl;
        cout << "  满分次数: " << simplePerfect << endl;
        cout << "  满分率: " << fixed << setprecision(2) << simpleRate << "%" << endl;

        cout << "\n复杂单人模式:" << endl;
        cout << "  总游戏次数: " << complexTotal << endl;
        cout << "  满分次数: " << complexPerfect << endl;
        cout << "  满分率: " << fixed << setprecision(2) << complexRate << "%" << endl;

        // 对战统计
        cout << "\n对战记录查询" << endl;
        cout << "请输入对手名字 (输入q返回): ";
        string opponent;
        cin >> opponent;

        if (opponent != "q")
        {
            // 检查对手是否存在
            ifstream userFile("userInfor.txt");
            bool opponentExist = false;
            string userline;
            while (getline(userFile, userline))
            {
                if (userline == opponent)
                {
                    opponentExist = true;
                    break;
                }
            }
            userFile.close();
            if (!opponentExist)
            {
                cout << "对手用户名不存在!" << endl;
                return;
            }
            // 统计对战记录
            ifstream matchFile("doublePlayerResult.txt");
            if (!matchFile.is_open())
            {
                cout << "无法打开对战记录文件!" << endl;
                return;
            }

            int totalMatches = 0;
            int wins = 0;

            string winner, loser;
            while (matchFile >> winner >> loser)
            {
                if ((winner == username && loser == opponent) ||
                    (winner == opponent && loser == username))
                {
                    totalMatches++;
                    if (winner == username)
                    {
                        wins++;
                    }
                }
            }
            matchFile.close();

            double winRate = totalMatches > 0 ? (double)wins / totalMatches * 100 : 0;

            cout << "\n与 " << opponent << " 的对战统计:" << endl;
            cout << "  总对战次数: " << totalMatches << endl;
            cout << "  胜利次数: " << wins << endl;
            cout << "  胜率: " << fixed << setprecision(2) << winRate << "%" << endl;
        }
    }
    catch (const exception &e)
    {
        cout << "查询统计信息时发生错误: " << e.what() << endl;
    }
}

// 显示游戏排名
void showGameRankings()
{
    try
    {
        // 用于存储玩家数据的结构
        struct PlayerData
        {
            string name;
            int simpleTotal = 0;
            int simplePerfect = 0;
            int complexTotal = 0;
            int complexPerfect = 0;
            int multiplayerWins = 0;
            int multiplayerTotal = 0;
        };

        // 存储所有玩家数据
        map<string, PlayerData> players;

        // 读取单人游戏数据
        ifstream scoreFile("scores.txt");
        if (!scoreFile.is_open())
        {
            cout << "无法打开分数文件!" << endl;
            return;
        }

        string username, mode;
        int score;
        while (scoreFile >> username >> mode >> score)
        {
            PlayerData &player = players[username];
            player.name = username;

            if (mode == "SIMPLE_SINGLE")
            {
                player.simpleTotal++;
                if (score == 1000)
                {
                    player.simplePerfect++;
                }
            }
            else if (mode == "COMPLEX_SINGLE")
            {
                player.complexTotal++;
                if (score == 1000)
                {
                    player.complexPerfect++;
                }
            }
        }
        scoreFile.close();

        // 读取多人游戏数据
        ifstream matchFile("doublePlayerResult.txt");
        if (matchFile.is_open())
        {
            string winner, loser;
            while (matchFile >> winner >> loser)
            {
                PlayerData &winnerData = players[winner];
                PlayerData &loserData = players[loser];

                winnerData.name = winner;
                loserData.name = loser;

                winnerData.multiplayerWins++;
                winnerData.multiplayerTotal++;
                loserData.multiplayerTotal++;
            }
            matchFile.close();
        }

        // 将玩家数据转换为可排序的向量
        vector<PlayerData> playerList;
        for (const auto &pair : players)
        {
            playerList.push_back(pair.second);
        }

        int choice;
        cout << "\n=== 排名查询 ===" << endl;
        cout << "1. 简单单人模式排名" << endl;
        cout << "2. 复杂单人模式排名" << endl;
        cout << "3. 多人模式胜率排名" << endl;
        cout << "请选择: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
        {
            // 使用自定义快速排序算法按满分率排序
            customQuickSort(playerList,
                            [](const PlayerData &a, const PlayerData &b)
                            {
                                double rateA = a.simpleTotal > 0 ? (double)a.simplePerfect / a.simpleTotal : 0;
                                double rateB = b.simpleTotal > 0 ? (double)b.simplePerfect / b.simpleTotal : 0;
                                return rateA > rateB;
                            });

            cout << "\n=== 简单单人模式排名 ===" << endl;
            cout << "排名\t玩家\t\t游戏次数\t满分率" << endl;

            for (size_t i = 0; i < playerList.size(); i++)
            {
                if (playerList[i].simpleTotal > 0)
                {
                    double rate = (double)playerList[i].simplePerfect / playerList[i].simpleTotal * 100;
                    cout << i + 1 << "\t" << playerList[i].name << "\t\t"
                         << playerList[i].simpleTotal << "\t\t"
                         << fixed << setprecision(2) << rate << "%" << endl;
                }
            }
            break;
        }
        case 2:
        {
            // 使用自定义快速排序算法按满分率排序
            customQuickSort(playerList,
                            [](const PlayerData &a, const PlayerData &b)
                            {
                                double rateA = a.complexTotal > 0 ? (double)a.complexPerfect / a.complexTotal : 0;
                                double rateB = b.complexTotal > 0 ? (double)b.complexPerfect / b.complexTotal : 0;
                                return rateA > rateB;
                            });

            cout << "\n=== 复杂单人模式排名 ===" << endl;
            cout << "排名\t玩家\t\t游戏次数\t满分率" << endl;

            for (size_t i = 0; i < playerList.size(); i++)
            {
                if (playerList[i].complexTotal > 0)
                {
                    double rate = (double)playerList[i].complexPerfect / playerList[i].complexTotal * 100;
                    cout << i + 1 << "\t" << playerList[i].name << "\t\t"
                         << playerList[i].complexTotal << "\t\t"
                         << fixed << setprecision(2) << rate << "%" << endl;
                }
            }
            break;
        }
        case 3:
        {
            // 使用自定义快速排序算法按多人模式胜率排序
            customQuickSort(playerList,
                            [](const PlayerData &a, const PlayerData &b)
                            {
                                double rateA = a.multiplayerTotal > 0 ? (double)a.multiplayerWins / a.multiplayerTotal : 0;
                                double rateB = b.multiplayerTotal > 0 ? (double)b.multiplayerWins / b.multiplayerTotal : 0;
                                return rateA > rateB;
                            });

            cout << "\n=== 多人模式胜率排名 ===" << endl;
            cout << "排名\t玩家\t\t总场次\t\t胜率" << endl;

            for (size_t i = 0; i < playerList.size(); i++)
            {
                if (playerList[i].multiplayerTotal > 0)
                {
                    double rate = (double)playerList[i].multiplayerWins / playerList[i].multiplayerTotal * 100;
                    cout << i + 1 << "\t" << playerList[i].name << "\t\t"
                         << playerList[i].multiplayerTotal << "\t\t"
                         << fixed << setprecision(2) << rate << "%" << endl;
                }
            }
            break;
        }
        default:
            cout << "无效选择!" << endl;
        }
    }
    catch (const exception &e)
    {
        cout << "查询排名信息时发生错误: " << e.what() << endl;
    }
}