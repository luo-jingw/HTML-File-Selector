#include <iostream>
#include <string>
#include <windows.h>
using namespace std;

bool openFile(const string& path) {
    string cmd = "start msedge \"" + path + "\"";
    int result = system(cmd.c_str());
    return (result == 0);
}

int main() {
    SetConsoleOutputCP(65001);
    
    while (true) {
        cout << "请选择要打开的文件：" << endl;
        cout << "1. ROS2 指南" << endl;
        cout << "2. LeetCode 练习" << endl;
        cout << "0. 退出程序" << endl;
        cout << "请输入选项 (0-2): ";
        
        int choice;
        cin >> choice;
        
        switch (choice) {
            case 0:
                return 0;
            case 1:
                if (openFile("\"E:\\guide_html\\ros2-guide.html\""))
                    return 0;
                break;
            case 2:
                if (openFile("\"E:\\guide_html\\leetcode.html\""))
                    return 0;
                break;
            default:
                cout << "无效选项，请重新选择！" << endl;
        }
        cout << "\n";
    }
    return 0;
}