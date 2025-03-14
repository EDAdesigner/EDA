#include "MyParser.h"
#include "strutil.h"
#include <assert.h> 

Token Lexer::NextToken() {
    while (isspace(input[position])) {
        ++position;
    }

    if (position >= input.length()) {
        return Token(TokenType::EndOfInput, "");
    }

    char currentChar = input[position++];

    if (isalpha(currentChar)) { // Identifier or Keyword
        string identifier;
        while (isalnum(currentChar) || currentChar == '_') {
            identifier += currentChar;
            if (position < input.length()) {
                currentChar = input[position++];
            }
            else {
                break;
            }
        }
        position--; // Rollback the last character read
        if (keywords.count(identifier)) {
            return Token(TokenType::Keyword, identifier);
        }
        else {
            return Token(TokenType::Identifier, identifier);
        }
    }
    else if (currentChar == ',' || currentChar == ';' || currentChar == '=' || currentChar == '&' || currentChar == '|' || currentChar == '(' || currentChar == ')') {
        return Token(TokenType::Symbol, string(1, currentChar));
    }

    return Token(TokenType::EndOfInput, ""); // Unknown token
}

string Lexer::GetWholeExpression()
{
    string exp;
    while (isspace(input[position])) {
        ++position;
    }
    auto startpos = position;
    while (input[position] != ';')
    {
        position++;
    }

    return input.substr(startpos, position - startpos);
}


void Parser::Parse() {
    auto moduleToken = lexer.NextToken();
    if (moduleToken.type != TokenType::Keyword || moduleToken.value != "module") {
        throw runtime_error("Expected 'module'");
    }

    auto moduleNameToken = lexer.NextToken();
    if (moduleNameToken.type != TokenType::Identifier) {
        throw runtime_error("Expected module name");
    }

    string moduleName = moduleNameToken.value;
    m_AstRoot = make_unique<ModuleNode>(moduleName);

    Module* pmod = new Module;
    pmod->modname = moduleName;
    m_module_map[moduleName] = pmod;

    // 解析参数
    auto token = lexer.NextToken();
    while (token.type != TokenType::EndOfInput && token.value != ";") { // 添加 EndOfInput 检查
        if (token.type == TokenType::Identifier) {
            PWire* param = new PWire;
            param->wiretype = PWire::PORT;
            param->name = token.value;
            pmod->wires[token.value] = param;
        }
        token = lexer.NextToken();
    }

    // 解析输入和输出端口
    token = lexer.NextToken();
    while (token.type != TokenType::EndOfInput && (token.type != TokenType::Keyword || token.value != "endmodule")) { // 添加 EndOfInput 检查
        if (token.type == TokenType::Keyword && token.value == "input") {
            token = lexer.NextToken();
            while (token.type != TokenType::EndOfInput && token.value != ";") { // 添加 EndOfInput 检查
                if (token.type == TokenType::Identifier) {
                    pmod->inputs.push_back(token.value);
                    unique_ptr<AstNode> portNode = make_unique<PortNode>(token.value, PortNode::INPUT);
                    m_AstRoot->AddStatement(move(portNode));
                }
                token = lexer.NextToken();
                if (token.value == ",") {
                    token = lexer.NextToken();
                }
            }
        }
        else if (token.type == TokenType::Keyword && token.value == "output") {
            token = lexer.NextToken();
            while (token.type != TokenType::EndOfInput && token.value != ";") { // 添加 EndOfInput 检查
                if (token.type == TokenType::Identifier) {
                    pmod->outputs.push_back(token.value);
                    unique_ptr<AstNode> portNode = make_unique<PortNode>(token.value, PortNode::OUTPUT);
                    m_AstRoot->AddStatement(move(portNode));
                }
                token = lexer.NextToken();
                if (token.value == ",") {
                    token = lexer.NextToken();
                }
            }
        }
        else {
            ParseStatement(pmod, token, m_AstRoot);
        }
        token = lexer.NextToken();
    }

    // 解析语句
    token = lexer.NextToken();
    while (token.type != TokenType::EndOfInput && (token.type != TokenType::Keyword || token.value != "endmodule")) { // 添加 EndOfInput 检查
        ParseStatement(pmod, token, m_AstRoot);
        token = lexer.NextToken();
    }
}

void  Parser::ParseStatement(Module* pmod,Token token, unique_ptr<ModuleNode>& AstRoot) {
    if (token.type == TokenType::Keyword && token.value == "wire") {
        auto wireToken = lexer.NextToken();
        if (wireToken.type != TokenType::Identifier) {
            throw runtime_error("Expected wire name");
        }
        PWire* param = new PWire;
        param->wiretype = PWire::MIDDLE;
        param->name = wireToken.value;
        pmod->wires[wireToken.value] = param;
    }
    else if (token.type == TokenType::Keyword && token.value == "assign") {
        PGAssign* ass = new PGAssign;
        ass->parser(pmod,lexer, AstRoot);
    }
}

void PGAssign::parser(Module* pmod, Lexer& lexer, unique_ptr<ModuleNode>& moduleNode) {
    auto lhsToken = lexer.NextToken();
    auto opToken = lexer.NextToken(); 
    string r_exp = lexer.GetWholeExpression();

    unique_ptr<AstNode> assnode;
    assnode = make_unique<AssignNode>(lhsToken.value, r_exp);
    moduleNode->AddStatement(move(assnode));

    // 解析右表达式
    char op;
    string lstr, rstr;
    splitByOperator(r_exp, lstr, rstr, op);

    // 根据操作符创建门级节点
    unique_ptr<AstNode> gateNode;
    if (op == '&') {
        gateNode = make_unique<AndGateNode>(lstr, rstr);
    }
    else if (op == '|') {
        gateNode = make_unique<OrGateNode>(lstr, rstr);
    }
    else {
        throw runtime_error("Unsupported gate operator: " + string(1, op));
    }

    moduleNode->AddStatement(move(gateNode));
}

void PExpr::parser(Module* pmod, Lexer& lexer, unique_ptr<ModuleNode>& moduleNode, string exp) {
    char op;
    string lstr, rstr;
    splitByOperator(exp, lstr, rstr, op);

    if (lstr.size() > 0)
    {
       //assert(opToken.value == lstr);
       if(!pmod->is_in_wires(lstr))
           throw runtime_error("undefine variable"+ lstr);
       moduleNode->AddStatement(make_unique<WireNode>(lstr));
    }
    if (rstr.size()> 0)
    {
       PExpr* r_val = new PEBinary;
       r_val->parser(pmod,lexer, moduleNode, rstr);
    }
}
