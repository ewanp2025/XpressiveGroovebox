#ifndef XPRESSIVE_PARSER_H
#define XPRESSIVE_PARSER_H

#include <string>
#include <memory>
#include <functional>

namespace XpressiveParser {
class Node; // Forward declaration

struct Env {
    double t=0, tempo=120, srate=44100, f=440;


    Node* w1 = nullptr;
    Node* w2 = nullptr;
    Node* w3 = nullptr;
};

class Node {
public:
    virtual ~Node()=default;
    virtual double eval(Env& env)=0;
};

std::unique_ptr<Node> parse(const std::string& str);
}

#endif // XPRESSIVE_PARSER_H