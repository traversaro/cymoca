//
// Created by jgoppert on 6/28/18.
//

#pragma once

#include "cymoca_compiler/ast/ast.h"
#include <sstream>

using namespace std;
using namespace cymoca::ast;

namespace cymoca {
namespace listener {

/**
 * A listener to print to quasi-lisp format.
 */
class LispPrinter : public ConstListener {
protected:
  stringstream _ss;

public:
  LispPrinter() : _ss("") {}
  void enterEvery(const Node &ctx) override { _ss << "("; }
  void exitEvery(const Node &ctx) override { _ss << ")"; }
  void enter(const Class &ctx) override { _ss << "class"; }
  void enter(const Dict<Component> &ctx) override { _ss << "dict of comp"; }
  void exit(const Dict<Component> &ctx) override { _ss << "\n"; }
  void enter(const Component &ctx) override {
    _ss << prefixStr[ctx.prefix()] << " " << ctx.name();
  }
  void enter(const ComponentRef &ctx) override { _ss << ctx.name(); }
  void enter(const List<Equation> &ctx) override { _ss << "list of eq"; }
  void exit(const List<Equation> &ctx) override { _ss << ""; }
  void enter(const Number &ctx) override { _ss << ctx.val(); }
  void enter(const UnaryExpr &ctx) override { _ss << unaryOpStr[ctx.op()]; }
  void enter(const SimpleEquation &ctx) override { _ss << "="; }
  void enter(const EquationBlock &ctx) override { _ss << "eq block"; }
  void enter(const FunctionCall &ctx) override { _ss << ctx.name(); }
  void enter(const StatementBlock &ctx) override { _ss << "stmt block"; }
  void enter(const WhenEquation &ctx) override { _ss << "when"; }
  void enter(const WhenStatement &ctx) override { _ss << "when"; }
  void enter(const IfEquation &ctx) override { _ss << "if"; }
  void enter(const IfStatement &ctx) override { _ss << "if"; }
  void enter(const Relation &ctx) override { _ss << relationOpStr[ctx.op()]; }
  void enter(const BinaryExpr &ctx) override { _ss << binaryOpStr[ctx.op()]; }
  void enter(const BinaryLogicExpr &ctx) override {
    _ss << binaryLogicOpStr[ctx.op()];
  }
  void enter(const UnaryLogicExpr &ctx) override {
    _ss << unaryLogicOpStr[ctx.op()];
  }
  string get() {
    string s = _ss.str();
    _ss.str("");
    return s;
  }
};

} // namespace listener
} // namespace cymoca

// vim: set et fenc=utf-8 ff=unix sts=0 sw=2 ts=2 :
