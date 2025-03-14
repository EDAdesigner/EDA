#include "strutil.h"
#include <iostream>

// 函数：将字符串按运算符划分为左右两个子字符串

std::string trim(const std::string& str) {
    // 去掉前导空格
    auto start = std::find_if(str.begin(), str.end(), [](unsigned char c) {
        return !std::isspace(c);
        });

    // 去掉尾部空格
    auto end = std::find_if(str.rbegin(), str.rend(), [](unsigned char c) {
        return !std::isspace(c);
        }).base();

        // 如果字符串全是空格，返回空字符串
        if (start >= end) {
            return "";
        }

        // 返回去掉前后空格的子字符串
        return std::string(start, end);
}

void splitByOperator(const string& str, string& left, string& right, char& op) {
    // 定义运算符集合
    string operators = "+-*/&|";

    // 遍历字符串，查找运算符
    for (size_t i = 0; i < str.size(); ++i) {
        if (operators.find(str[i]) != string::npos) {
            // 找到运算符，分割字符串
            op = str[i];                    //运算符
            left = trim(str.substr(0, i));       // 左子字符串
            right = trim(str.substr(i + 1));     // 右子字符串
            return;
        }
    }

    // 如果没有找到运算符，返回空字符串
    left = str;
    right = "";
}