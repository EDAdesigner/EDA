#pragma once
#ifndef PARSERFORM_H
#define PARSERFORM_H

#include <map>
#include <string>
#include <list>
#include <vector>

using namespace std;

struct Module;
struct PWire;
struct PGate;
struct PGAssign;
struct PExpr;
struct PEBinary;
struct PEUnary;
struct PEIdent;

struct Module {
    map<string, PWire*> wires;
    list<PGate*> gates;

    const list<PGate*>& get_gates() const {
        return gates;
    }
    bool is_in_wires(string& name) {
        if (wires.find(name) != wires.end()) {
            return true;
        }
        return false;
    }
    string* parser();
    void print();
};

struct PWire {
    enum WireType {
        PORT, //输入输出
        MIDDLE,//声明中间变量
    };
    string name;
    WireType port_type_;
    WireType get_port_type() const { return port_type_; }
    void parser();
    void print();
};

struct PGate {
    virtual void genTAC(MidCode& mcode);
    virtual void parser();
    virtual void print();

};

struct PExpr {
    //virtual Value* genTAC(MidCode& mcode);
    virtual void parser(Lexer& lexer);
    //virtual void print();
};

struct PGAssign : public PGate {
    vector<PExpr*> pins_;

    vector<PExpr*>& get_pins() { return pins_; }
    void genTAC(MidCode& mcode);
    void parser();
    void print();
};

struct PEBinary : public PExpr {
    PExpr* left_;
    PExpr* right_;
    char op_;
    Value* genTAC(MidCode& mcode);
    void parser();
    void print();
};

struct PEUnary : public PExpr {
    char op_;
    PExpr* expr_;
    Value* genTAC(MidCode& mcode);
    void parser();
    void print();
};

struct PEIdent : public PExpr {
    string name;
    Value* genTAC(MidCode& mcode);
    void parser();
    void print();
};

#endif