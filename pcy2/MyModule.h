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
        PORT, //�������
        MIDDLE,//�����м����
    };
    string name;
    WireType wiretype;
    WireType get_port_type() const { return wiretype; }
    void parser();
    void print();
};

// ģ�������ڵ�
struct Module {
    string modname;

    std::vector<std::string> inputs;  // ����˿�
    std::vector<std::string> outputs; // ����˿�
    
    map<string, PWire*> wires;
    //string* parser();
    //void print();
    bool is_in_wires(string& name);
};

#endif

