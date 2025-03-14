#pragma once
#ifndef MYPARSER_H
#define MYPARSER_H

#include "MyModule.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
// AST�ڵ����
class AstNode {
public:
    virtual ~AstNode() = default;
    virtual void Print(int depth) const {} // ����麯��
};

class ModuleNode : public AstNode {
public:
    string name;
    //vector<pair<string, bool>> ports; // pair: <port_name, is_input>
    vector<unique_ptr<AstNode>> statements;

    ModuleNode(string name) : name(move(name)) {}

    void AddStatement(unique_ptr<AstNode> statement) {
        statements.push_back(move(statement));
    }
};

// ��ֵ���ڵ�
class AssignNode : public AstNode {
public:
    string lhs;
    string rhs;

    AssignNode(string lhs, string rhs) : lhs(move(lhs)), rhs(move(rhs)) {}
};

// ���������ڵ�
class WireNode : public AstNode {
public:
    string name;

    WireNode(string name) : name(move(name)) {}
};


// ����ʷ���Ԫ������
enum class TokenType {
    Identifier,
    Keyword,
    Symbol,
    EndOfInput
};

// �ʷ���Ԫ��
class Token {
public:
    TokenType type;
    string value;

    Token(TokenType type, string value) : type(type), value(move(value)) {}
};

// �ʷ�������
class Lexer {
public:
    explicit Lexer(const string& input) : input(input), position(0) {}
    Token NextToken();
    string GetWholeExpression();
    //Token GetTokenFromSubExpression(string str);
private:
    const string input;
    size_t position;
    unordered_set<string> keywords = { "module", "input", "output", "wire", "assign", "endmodule" };
};

// �﷨������
class Parser {
public:
    explicit Parser(Lexer& lexer) : lexer(lexer) {}
    void Parse();
    void  ParseStatement(Module* rmod,Token token, unique_ptr<ModuleNode>& moduleNode);
    bool is_in_wires(string& name);
    map<string, Module*> m_module_map;
    unique_ptr<ModuleNode> m_AstRoot;
private:
    Lexer& lexer;
};


struct PExpr {

    virtual void parser(Module* pmod, Lexer& lexer, unique_ptr<ModuleNode>& moduleNode, string exp);
    //virtual void print();
};

struct PEBinary : public PExpr {
    PExpr* left_;
    PExpr* right_;
    char op_;
    //void parser();
    //void print();
};

struct PGAssign {
    void parser(Module* pmod, Lexer& lexer, unique_ptr<ModuleNode>& moduleNode);
    //void print();
};


class PortNode : public AstNode {
public:
    enum PortType { INPUT, OUTPUT };

    PortNode(const std::string& name, PortType type) : name(name), type(type) {}

    void Print(int depth) const override { // ʹ�� override �ؼ���
        std::cout << std::string(depth * 2, ' ') << (type == INPUT ? "Input: " : "Output: ") << name << std::endl;
    }

private:
    std::string name;
    PortType type;
};

class AndGateNode : public AstNode {
public:
    AndGateNode(const std::string& lhs, const std::string& rhs) : lhs(lhs), rhs(rhs) {}

    void Print(int depth) const override { // ʹ�� override �ؼ���
        std::cout << std::string(depth * 2, ' ') << "AndGate: " << lhs << " & " << rhs << std::endl;
    }

private:
    std::string lhs, rhs;
};

class OrGateNode : public AstNode {
public:
    OrGateNode(const std::string& lhs, const std::string& rhs) : lhs(lhs), rhs(rhs) {}

    void Print(int depth) const override { // ʹ�� override �ؼ���
        std::cout << std::string(depth * 2, ' ') << "OrGate: " << lhs << " | " << rhs << std::endl;
    }

private:
    std::string lhs, rhs;
};

#endif