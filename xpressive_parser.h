#ifndef XPRESSIVE_PARSER_H
#define XPRESSIVE_PARSER_H

#include <string>
#include <memory>
#include <functional>

namespace XpressiveParser {
    struct Env { double t=0, tempo=120, srate=44100, f=440; };
    class Node { public: virtual ~Node()=default; virtual double eval(Env& env)=0; };


    std::unique_ptr<Node> parse(const std::string& str);
}

#endif // XPRESSIVE_PARSER_H