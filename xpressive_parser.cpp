#include "xpressive_parser.h"
#include <cmath>
#include <stdexcept>
#include <cctype>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace XpressiveParser {

enum Token { TOK_EOF, TOK_NUM, TOK_ID, TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV, TOK_LPAREN, TOK_RPAREN, TOK_COMMA, TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LTE, TOK_GTE, TOK_OR, TOK_AND };

struct Lexer {
    const char* p; Token curTok; double numVal; std::string idStr;
    Lexer(const char* str) : p(str) { next(); }
    void next() {
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        if (!*p) { curTok = TOK_EOF; return; }
        if (isdigit(*p) || *p == '.') { char* end; numVal = strtod(p, &end); p = end; curTok = TOK_NUM; return; }
        if (isalpha(*p)) { idStr.clear(); while (isalnum(*p) || *p == '_') idStr += *p++; curTok = TOK_ID; return; }
        char c = *p++;
        if (c == '=') { if (*p == '=') { p++; curTok = TOK_EQ; } else curTok = TOK_EOF; }
        else if (c == '!') { if (*p == '=') { p++; curTok = TOK_NEQ; } else curTok = TOK_EOF; }
        else if (c == '<') { if (*p == '=') { p++; curTok = TOK_LTE; } else curTok = TOK_LT; }
        else if (c == '>') { if (*p == '=') { p++; curTok = TOK_GTE; } else curTok = TOK_GT; }
        else if (c == '|') { curTok = TOK_OR; } else if (c == '&') { curTok = TOK_AND; }
        else if (c == '+') curTok = TOK_PLUS; else if (c == '-') curTok = TOK_MINUS;
        else if (c == '*') curTok = TOK_MUL; else if (c == '/') curTok = TOK_DIV;
        else if (c == '(') curTok = TOK_LPAREN; else if (c == ')') curTok = TOK_RPAREN;
        else if (c == ',') curTok = TOK_COMMA; else curTok = TOK_EOF;
    }
};

class NumNode : public Node { double val; public: NumNode(double v):val(v){} double eval(Env&) override { return val; } };
class VarNode : public Node { std::string name; public: VarNode(std::string n):name(n){} double eval(Env& e) override { if(name=="t") return e.t; if(name=="tempo") return e.tempo; if(name=="srate") return e.srate; return e.f; } };
class UnaryNode : public Node { std::unique_ptr<Node> n; std::function<double(double)> op; public: UnaryNode(std::unique_ptr<Node> n, std::function<double(double)> op):n(std::move(n)),op(op){} double eval(Env& e) override { return op(n->eval(e)); } };
class FuncNode : public Node { std::unique_ptr<Node> n; std::function<double(double)> op; public: FuncNode(std::unique_ptr<Node> n, std::function<double(double)> op):n(std::move(n)),op(op){} double eval(Env& e) override { return op(n->eval(e)); } };
class BinFuncNode : public Node { std::unique_ptr<Node> l,r; std::function<double(double,double)> op; public: BinFuncNode(std::unique_ptr<Node> l, std::unique_ptr<Node> r, std::function<double(double,double)> op):l(std::move(l)),r(std::move(r)),op(op){} double eval(Env& e) override { return op(l->eval(e), r->eval(e)); } };
class TernaryFuncNode : public Node { std::unique_ptr<Node> a,b,c; std::function<double(double,double,double)> op; public: TernaryFuncNode(std::unique_ptr<Node> a, std::unique_ptr<Node> b, std::unique_ptr<Node> c, std::function<double(double,double,double)> op):a(std::move(a)),b(std::move(b)),c(std::move(c)),op(op){} double eval(Env& e) override { return op(a->eval(e), b->eval(e), c->eval(e)); } };
class BinOpNode : public Node { std::unique_ptr<Node> l,r; Token op; public: BinOpNode(std::unique_ptr<Node> l, std::unique_ptr<Node> r, Token op):l(std::move(l)),r(std::move(r)),op(op){} double eval(Env& e) override { double a=l->eval(e), b=r->eval(e); switch(op){ case TOK_PLUS: return a+b; case TOK_MINUS: return a-b; case TOK_MUL: return a*b; case TOK_DIV: return b==0?0:a/b; case TOK_EQ: return a==b?1:0; case TOK_NEQ: return a!=b?1:0; case TOK_LT: return a<b?1:0; case TOK_GT: return a>b?1:0; case TOK_LTE: return a<=b?1:0; case TOK_GTE: return a>=b?1:0; case TOK_OR: return (a!=0||b!=0)?1:0; case TOK_AND: return (a!=0&&b!=0)?1:0; default: return 0; } } };

class IntegrateNode : public Node { std::unique_ptr<Node> n; double accum=0; public: IntegrateNode(std::unique_ptr<Node> n):n(std::move(n)){} double eval(Env& e) override { accum += n->eval(e) / e.srate; return accum; } };

std::unique_ptr<Node> parseExpr(Lexer& lex);
std::unique_ptr<Node> parsePrimary(Lexer& lex) {
    if (lex.curTok == TOK_NUM) { auto n = std::make_unique<NumNode>(lex.numVal); lex.next(); return n; }
    if (lex.curTok == TOK_ID) {
        std::string id = lex.idStr; lex.next();
        if (id == "t" || id == "tempo" || id == "srate" || id == "f") return std::make_unique<VarNode>(id);
        if (lex.curTok == TOK_LPAREN) {
            lex.next(); auto a1 = parseExpr(lex);

            if (id == "clamp") {
                if (lex.curTok != TOK_COMMA) throw std::runtime_error("Expected ',' in clamp");
                lex.next(); auto a2 = parseExpr(lex);
                if (lex.curTok != TOK_COMMA) throw std::runtime_error("Expected ',' in clamp");
                lex.next(); auto a3 = parseExpr(lex);
                if (lex.curTok != TOK_RPAREN) throw std::runtime_error("Expected ')' in clamp");
                lex.next();
                return std::make_unique<TernaryFuncNode>(std::move(a1), std::move(a2), std::move(a3), [](double mn, double v, double mx){ return std::max(mn, std::min(v, mx)); });
            }

            std::unique_ptr<Node> a2;
            if (lex.curTok == TOK_COMMA) { lex.next(); a2 = parseExpr(lex); }
            if (lex.curTok != TOK_RPAREN) throw std::runtime_error("Expected ')'");
            lex.next();

            if (id == "mod") return std::make_unique<BinFuncNode>(std::move(a1), std::move(a2), [](double a, double b){ return b==0?0:std::fmod(a,b); });
            if (id == "sinew") return std::make_unique<FuncNode>(std::move(a1), [](double x){ return std::sin(2.0*M_PI*x); });
            if (id == "saww") return std::make_unique<FuncNode>(std::move(a1), [](double x){ double p=x-std::floor(x); return 2.0*p-1.0; });
            if (id == "squarew") return std::make_unique<FuncNode>(std::move(a1), [](double x){ double p=x-std::floor(x); return p<0.5?1.0:-1.0; });
            if (id == "exp") return std::make_unique<FuncNode>(std::move(a1), [](double x){ return std::exp(x); });
            if (id == "floor") return std::make_unique<FuncNode>(std::move(a1), [](double x){ return std::floor(x); });
            if (id == "randv") return std::make_unique<FuncNode>(std::move(a1), [](double){ return (static_cast<double>(rand())/RAND_MAX)*2.0-1.0; });
            if (id == "integrate") return std::make_unique<IntegrateNode>(std::move(a1));

            if (id == "W1") return std::make_unique<FuncNode>(std::move(a1), [](double x){
                    double p1 = x - std::floor(x);
                    double p2 = (x*2.0) - std::floor(x*2.0);
                    return std::max(-1.0, std::min((2.0*p1-1.0) + 0.5*(p2<0.5?1.0:-1.0), 1.0));
                });
            if (id == "W2") return std::make_unique<FuncNode>(std::move(a1), [](double x){
                    return 0.3 * std::sin(2.0*M_PI*x*0.5);
                });
        }
        throw std::runtime_error("Unknown function: " + id);
    }
    if (lex.curTok == TOK_LPAREN) { lex.next(); auto n = parseExpr(lex); if (lex.curTok != TOK_RPAREN) throw std::runtime_error("Expected ')'"); lex.next(); return n; }
    throw std::runtime_error("Syntax error");
}
std::unique_ptr<Node> parseUnary(Lexer& lex) { if(lex.curTok==TOK_MINUS){ lex.next(); return std::make_unique<UnaryNode>(parsePrimary(lex), [](double x){return -x;}); } return parsePrimary(lex); }
std::unique_ptr<Node> parseMulDiv(Lexer& lex) { auto l=parseUnary(lex); while(lex.curTok==TOK_MUL||lex.curTok==TOK_DIV){ Token op=lex.curTok; lex.next(); l=std::make_unique<BinOpNode>(std::move(l), parseUnary(lex), op); } return l; }
std::unique_ptr<Node> parseAddSub(Lexer& lex) { auto l=parseMulDiv(lex); while(lex.curTok==TOK_PLUS||lex.curTok==TOK_MINUS){ Token op=lex.curTok; lex.next(); l=std::make_unique<BinOpNode>(std::move(l), parseMulDiv(lex), op); } return l; }
std::unique_ptr<Node> parseRel(Lexer& lex) { auto l=parseAddSub(lex); while(lex.curTok==TOK_LT||lex.curTok==TOK_GT||lex.curTok==TOK_LTE||lex.curTok==TOK_GTE){ Token op=lex.curTok; lex.next(); l=std::make_unique<BinOpNode>(std::move(l), parseAddSub(lex), op); } return l; }
std::unique_ptr<Node> parseEq(Lexer& lex) { auto l=parseRel(lex); while(lex.curTok==TOK_EQ||lex.curTok==TOK_NEQ){ Token op=lex.curTok; lex.next(); l=std::make_unique<BinOpNode>(std::move(l), parseRel(lex), op); } return l; }
std::unique_ptr<Node> parseAnd(Lexer& lex) { auto l=parseEq(lex); while(lex.curTok==TOK_AND){ lex.next(); l=std::make_unique<BinOpNode>(std::move(l), parseEq(lex), TOK_AND); } return l; }
std::unique_ptr<Node> parseExpr(Lexer& lex) { auto l=parseAnd(lex); while(lex.curTok==TOK_OR){ lex.next(); l=std::make_unique<BinOpNode>(std::move(l), parseAnd(lex), TOK_OR); } return l; }
std::unique_ptr<Node> parse(const std::string& str) { Lexer lex(str.c_str()); return parseExpr(lex); }

} // namespace XpressiveParser