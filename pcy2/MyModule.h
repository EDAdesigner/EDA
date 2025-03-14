#pragma once
#ifndef MYMODULE_H
#define MYMODULE_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <cctype>
#include <map>
using namespace std;

struct PWire {
    enum WireType {
        PORT, //输入输出
        MIDDLE,//声明中间变量
    };
    string name;
    WireType wiretype;
    WireType get_port_type() const { return wiretype; }
    void parser();
    void print();
};

// 模块声明节点
struct Module {
    string modname;

    std::vector<std::string> inputs;  // 输入端口
    std::vector<std::string> outputs; // 输出端口
    
    map<string, PWire*> wires;
    //string* parser();
    //void print();
    bool is_in_wires(string& name);
};

#endif

