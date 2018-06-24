//
// Created by jgoppert on 3/17/18.
//

#include "Compiler.h"
#include "cymoca_compiler/listener/Lisp.h"

namespace cymoca {

Compiler::Compiler(std::ifstream &text) :
    _parser(nullptr),
    _input(text),
    _lexer(&_input),
    _tokenStream(&_lexer),
    _root(nullptr),
    _verbose(false),
    _ast() {
  _tokenStream.fill();
  _parser = std::make_shared<ModelicaParser>(&_tokenStream);
  antlr4::tree::ParseTree *tree = _parser->stored_definition();
  antlr4::tree::ParseTreeWalker::DEFAULT.walk(this, tree);
}

std::string Compiler::indent(int n) {
  std::string s;
  for (int i = 0; i < n; i++) {
    s.append(" ");
  }
  return s;
}

void Compiler::visitTerminal(antlr4::tree::TerminalNode *node) {
  if (_verbose) {
    std::cout << "terminal: " << node->getText() << std::endl;
  }
}

void Compiler::visitErrorNode(antlr4::tree::ErrorNode *node) {
}

void Compiler::enterEveryRule(antlr4::ParserRuleContext *ctx) {
  if (_verbose) {
    std::cout << indent(ctx->depth()) << ">> " << _parser->getRuleNames()[ctx->getRuleIndex()] << std::endl;
  }
}

void Compiler::exitEveryRule(antlr4::ParserRuleContext *ctx) {
  if (_verbose) {
    std::cout << indent(ctx->depth()) << "<< " << _parser->getRuleNames()[ctx->getRuleIndex()] << std::endl;
  }
}

void Compiler::exitPrimary_unsigned_number(ModelicaParser::Primary_unsigned_numberContext *ctx) {
  std::stringstream ss(ctx->getText());
  double num;
  ss >> num;
  _ast[ctx] = std::make_shared<ast::UnsignedNumber>(num);
}

void Compiler::exitExpr_negative(ModelicaParser::Expr_negativeContext *ctx) {
  auto e = getAst<ast::Expr>(ctx->expr());
  _ast[ctx] = std::make_shared<ast::Negative>(e);
}

void Compiler::exitPrimary_component_reference(ModelicaParser::Primary_component_referenceContext *ctx) {
  // TODO handle mutliple levels of naming in IDENT
  _ast[ctx] = std::make_shared<ast::ComponentRef>(ctx->component_reference()->getText());
}

void Compiler::exitComponent_declaration(ModelicaParser::Component_declarationContext *ctx) {
  _ast[ctx] = std::make_shared<ast::Component>(ctx->declaration()->getText());
}

void Compiler::exitComposition(ModelicaParser::CompositionContext *ctx) {
  auto c = std::make_shared<ast::Class>();

  //for (auto &elem_list: ctx->element_list()) {
    //for (auto &elem: elem_list->element()) {
    //}
  //}

  for (auto &eq_sec: ctx->equation_section()) {
    auto sec = getAst<ast::EquationList>(eq_sec->equation_list());
    c->addEquationSection(sec);
  }

  //for (auto &alg_sec: ctx->algorithm_section()) {
  //  for (auto &stmt: alg_sec->statement_list()->statement()) {
  //  }
  //}
  _root = c;
}

void Compiler::exitExpression_simple(ModelicaParser::Expression_simpleContext *ctx) {
  linkAst(ctx, ctx->simple_expression());
}

void Compiler::exitSimple_expression(ModelicaParser::Simple_expressionContext *ctx) {
  // TODO handle other : expr's
  linkAst(ctx, ctx->expr(0));
}

void Compiler::exitEquation_simple(ModelicaParser::Equation_simpleContext *ctx) {
  auto left = getAst<ast::Expr>(ctx->simple_expression());
  auto right = getAst<ast::Expr>(ctx->expression());
  setAst(ctx, std::make_shared<ast::Equation>(left, right));
}

void Compiler::exitWhen_equation(ModelicaParser::When_equationContext *ctx) {
  auto whenEq = std::make_shared<ast::WhenEquation>();
  for (size_t i = 0; i < ctx->equation_list().size(); i++) {
    auto eqList = getAst<ast::EquationList>(ctx->equation_list(i));
    if (i < ctx->expression().size()) {
      auto cond = getAst<ast::Expr>(ctx->expression(i));
      whenEq->addBlock(cond, eqList);
    } else {
      whenEq->addBlock(std::make_shared<ast::Boolean>(true), eqList);
    }
  }
  setAst(ctx, whenEq);
}

void Compiler::exitEquation_list(ModelicaParser::Equation_listContext *ctx) {
  auto eqList = std::make_shared<ast::EquationList>();
  for (auto &eq: ctx->equation()) {
    eqList->addEquation(getAst<ast::Node>(eq));
  }
  setAst(ctx, eqList);
}

void Compiler::exitPrimary_der(ModelicaParser::Primary_derContext *ctx) {
  auto var = getAst<ast::Expr>(ctx->function_call_args()->function_arguments());
  setAst(ctx, std::make_shared<ast::Derivative>(var));
}

void Compiler::exitArgs_expression(ModelicaParser::Args_expressionContext *ctx) {
  linkAst(ctx, ctx->expression());
}

void Compiler::exitExpr_relation(ModelicaParser::Expr_relationContext *ctx) {
  auto left = getAst<ast::Expr>(ctx->expr(0));
  auto right = getAst<ast::Expr>(ctx->expr(1));
  setAst(ctx, std::make_shared<ast::Relation>(left, ctx->op->getText(), right));
}

void Compiler::exitPrimary_output_expression_list(ModelicaParser::Primary_output_expression_listContext *ctx) {
  linkAst(ctx, ctx->output_expression_list());
}

void Compiler::exitOutput_expression_list(ModelicaParser::Output_expression_listContext *ctx) {
  // TODO handle other items in list
  linkAst(ctx, ctx->expression(0));
}

void Compiler::exitEquation(ModelicaParser::EquationContext *ctx) {
  linkAst(ctx, ctx->equation_options());
}

void Compiler::exitEquation_options(ModelicaParser::Equation_optionsContext *ctx) {
  linkAst(ctx, dynamic_cast<antlr4::ParserRuleContext *>(ctx->children[0]));
}

void Compiler::exitStatement_options(ModelicaParser::Statement_optionsContext *ctx) {
  linkAst(ctx, dynamic_cast<antlr4::ParserRuleContext *>(ctx->children[0]));
}

void Compiler::exitIf_equation(ModelicaParser::If_equationContext *ctx) {
  auto ifEq = std::make_shared<ast::IfEquation>();
  for (size_t i = 0; i < ctx->equation_list().size(); i++) {
    auto eqList = getAst<ast::EquationList>(ctx->equation_list(i));
    if (i < ctx->expression().size()) {
      auto cond = getAst<ast::Expr>(ctx->expression(i));
      ifEq->addBlock(std::make_shared<ast::EquationBlock>(cond, eqList));
    } else {
      ifEq->addBlock(std::make_shared<ast::EquationBlock>(
          std::make_shared<ast::Boolean>(true), eqList));
    }
  }
  setAst(ctx, ifEq);}
} // cymoca
