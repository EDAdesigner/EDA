#include "MyParser.h"
#include <iostream>
#include <fstream>
#include <sstream>

// 打印 AST
void PrintAst(const AstNode* node, int depth = 0) {
    if (dynamic_cast<const ModuleNode*>(node)) {
        const auto& moduleNode = dynamic_cast<const ModuleNode*>(node);
        std::cout << std::string(depth * 2, ' ') << "Module: " << moduleNode->name << std::endl;
        for (const auto& statement : moduleNode->statements) {
            PrintAst(statement.get(), depth + 1);
        }
    }
    else if (dynamic_cast<const AssignNode*>(node)) {
        const auto& assignNode = dynamic_cast<const AssignNode*>(node);
        std::cout << std::string(depth * 2, ' ') << "Assign: " << assignNode->lhs << " = " << assignNode->rhs << std::endl;
    }
    else if (dynamic_cast<const WireNode*>(node)) {
        const auto& wireNode = dynamic_cast<const WireNode*>(node);
        std::cout << std::string(depth * 2, ' ') << "Wire: " << wireNode->name << std::endl;
    }
    else if (dynamic_cast<const PortNode*>(node)) {
        const auto& portNode = dynamic_cast<const PortNode*>(node);
        portNode->Print(depth); // 调用 PortNode 的 Print 函数
    }
    else if (dynamic_cast<const AndGateNode*>(node)) {
        const auto& andGateNode = dynamic_cast<const AndGateNode*>(node);
        andGateNode->Print(depth); // 调用 AndGateNode 的 Print 函数
    }
    else if (dynamic_cast<const OrGateNode*>(node)) {
        const auto& orGateNode = dynamic_cast<const OrGateNode*>(node);
        orGateNode->Print(depth); // 调用 OrGateNode 的 Print 函数
    }
}

// 从文件中读取内容
std::string ReadFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    std::string filePath = "test1.v"; // 直接指定文件名
    std::string input;

    try {
        input = ReadFile(filePath); // 读取文件内容
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    // 解析输入
    Lexer lexer(input);
    Parser parser(lexer);

    try {
        parser.Parse();
        PrintAst(parser.m_AstRoot.get());
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}