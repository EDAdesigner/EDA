#include "strutil.h"
#include <iostream>

// ���������ַ��������������Ϊ�����������ַ���

std::string trim(const std::string& str) {
    // ȥ��ǰ���ո�
    auto start = std::find_if(str.begin(), str.end(), [](unsigned char c) {
        return !std::isspace(c);
        });

    // ȥ��β���ո�
    auto end = std::find_if(str.rbegin(), str.rend(), [](unsigned char c) {
        return !std::isspace(c);
        }).base();

        // ����ַ���ȫ�ǿո񣬷��ؿ��ַ���
        if (start >= end) {
            return "";
        }

        // ����ȥ��ǰ��ո�����ַ���
        return std::string(start, end);
}

void splitByOperator(const string& str, string& left, string& right, char& op) {
    // �������������
    string operators = "+-*/&|";

    // �����ַ��������������
    for (size_t i = 0; i < str.size(); ++i) {
        if (operators.find(str[i]) != string::npos) {
            // �ҵ���������ָ��ַ���
            op = str[i];                    //�����
            left = trim(str.substr(0, i));       // �����ַ���
            right = trim(str.substr(i + 1));     // �����ַ���
            return;
        }
    }

    // ���û���ҵ�����������ؿ��ַ���
    left = str;
    right = "";
}