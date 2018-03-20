//
// Created by jgoppert on 3/17/18.
//

#ifndef CYMOCA_COMPILER_H
#define CYMOCA_COMPILER_H

#include <modelica_antlr/ModelicaBaseListener.h>
#include <modelica_antlr/ModelicaLexer.h>
#include <modelica_xsd/Modelica.hxx>
#include <typeindex>
#include <memory>

namespace ast = modelica_xsd;

using namespace modelica_antlr;

namespace cymoca {

typedef ::xml_schema::Type XmlBase;

/**
 * A simple structure to hold the base xml pointer
 * and type information about the derived class
 */
class AstData {
 protected:
  std::type_index _type;
  XmlBase *_xml;
 public:
  const std::type_index &getType() {
    return _type;
  }
  XmlBase * getXml() {
    return _xml;
  }
  AstData() : _type(typeid(nullptr)), _xml(nullptr) {
  }
  AstData(const std::type_index &type, XmlBase * xml) :
      _type(type),
      _xml(xml) {
  }
};

typedef std::unordered_map<antlr4::tree::ParseTree *, std::shared_ptr<AstData>> AstMap;
typedef std::unordered_map<antlr4::tree::ParseTree *, std::shared_ptr<XmlBase>> AstMemory;

/**
 * This is the main compiler class.
 */
class Compiler : public ModelicaListener {
 public:
  explicit Compiler(std::ifstream &text);
  ModelicaParser &getParser() { return *_parser; }
  antlr4::CommonTokenStream &getTokenStream() { return _tokenStream; }
  ModelicaParser::Stored_definitionContext *getRoot() { return _root; }
  AstMap &getAst() { return _ast; };
  void printXML(std::ostream &out);

  /**
   * Pretty print the parse tree and annotate it with xml model types
   * @return pretty string
   */
  std::string toPrettyStringTree();

  /**
   * Get the xml type information for the context
   * @tparam xmlType  the type from modelica_xsd
   * @param ctx the context
   * @return the type information
   */
  const std::type_index &getXmlType(antlr4::tree::ParseTree *ctx) {
    auto data = _ast[ctx];
    assert(data != nullptr);
    return _ast[ctx]->getType();
  };

  /**
   * Check if annotation data exists for a given context
   * @param ctx the context
   * @return true if data exists
   */
  bool hasData(antlr4::tree::ParseTree *ctx) {
    auto data = _ast[ctx];
    return data != nullptr;
  };

  /**
   * Get the xml model for the context, downcast base ptr
   * @tparam xmlType  the type from modelica_xsd
   * @param ctx the context from antlr
   * @return a reference to the xml
   */
  template<class xmlType>
  xmlType * getXml(antlr4::tree::ParseTree *ctx) {
    xmlType * a;
    std::shared_ptr<AstData> data = _ast[ctx];
    assert(data->getType() == typeid(a));
    XmlBase * xmlBase = data->getXml();
    assert(xmlBase != nullptr);
    auto xml = dynamic_cast<xmlType*>(xmlBase);
    assert(xml != nullptr);
    return xml;
  };

  void setMemory(antlr4::tree::ParseTree * ctx, std::shared_ptr<XmlBase> xml) {
    _memory[ctx] = xml;
  }

 protected:
  antlr4::ANTLRInputStream _input;
  std::shared_ptr<ModelicaParser> _parser;
  ModelicaLexer _lexer;
  antlr4::CommonTokenStream _tokenStream;
  AstMap _ast;
  AstMemory _memory;
  ModelicaParser::Stored_definitionContext *_root;

  /**
   * Annotate the AST with an xml model
   * @tparam xmlType the type from modelica_xsd
   * @param ctx  the context from antlr
   * @param store store the xml data
   * @return a pointer to the AstData
   */
  template<class xmlType>
  void setData(antlr4::tree::ParseTree *ctx, xmlType * xml) {
    auto data = std::make_shared<AstData>(typeid(xml), xml);
    assert(data != nullptr);
    _ast[ctx] = data;
  };

  /**
   * Annotate the AST with another annotation
   * this avoids unncessary duplication
   * @tparam xmlType the type from modelica_xsd
   * @param to context to link
   * @param from source of data
   * @return a pointer to the AstData
   */
  void linkData(antlr4::tree::ParseTree *to,
                antlr4::tree::ParseTree *from) {
    auto from_ptr = _ast[from];
    // ok if from is a nullptr
    // to should also become a nullptr
    _ast[to] = from_ptr;
  };

  /**
   * Get the xml annotation data
   * @tparam xmlType  the type from modelica_xsd
   * @param ctx the context from antlr
   * @return the AstData
   */
  std::shared_ptr<AstData> getData(antlr4::tree::ParseTree *ctx) {
    std::shared_ptr<AstData> data = _ast[ctx];
    assert(data != nullptr);
    return data;
  };

  /**
   * Listener Functions
   */

  void visitTerminal(antlr4::tree::TerminalNode *node) override;
  void visitErrorNode(antlr4::tree::ErrorNode *node) override;
  void enterEveryRule(antlr4::ParserRuleContext *ctx) override;
  void exitEveryRule(antlr4::ParserRuleContext *ctx) override;
  void enterStored_definition(ModelicaParser::Stored_definitionContext *ctx) override;
  void exitStored_definition(ModelicaParser::Stored_definitionContext *ctx) override;
  void enterStored_definition_class(ModelicaParser::Stored_definition_classContext *ctx) override;
  void exitStored_definition_class(ModelicaParser::Stored_definition_classContext *ctx) override;
  void enterClass_definition(ModelicaParser::Class_definitionContext *ctx) override;
  void exitClass_definition(ModelicaParser::Class_definitionContext *ctx) override;
  void enterClass_prefixes(ModelicaParser::Class_prefixesContext *ctx) override;
  void exitClass_prefixes(ModelicaParser::Class_prefixesContext *ctx) override;
  void enterClass_type(ModelicaParser::Class_typeContext *ctx) override;
  void exitClass_type(ModelicaParser::Class_typeContext *ctx) override;
  void enterClass_spec_comp(ModelicaParser::Class_spec_compContext *ctx) override;
  void exitClass_spec_comp(ModelicaParser::Class_spec_compContext *ctx) override;
  void enterClass_spec_base(ModelicaParser::Class_spec_baseContext *ctx) override;
  void exitClass_spec_base(ModelicaParser::Class_spec_baseContext *ctx) override;
  void enterClass_spec_enum(ModelicaParser::Class_spec_enumContext *ctx) override;
  void exitClass_spec_enum(ModelicaParser::Class_spec_enumContext *ctx) override;
  void enterClass_spec_der(ModelicaParser::Class_spec_derContext *ctx) override;
  void exitClass_spec_der(ModelicaParser::Class_spec_derContext *ctx) override;
  void enterClass_spec_extends(ModelicaParser::Class_spec_extendsContext *ctx) override;
  void exitClass_spec_extends(ModelicaParser::Class_spec_extendsContext *ctx) override;
  void enterBase_prefix(ModelicaParser::Base_prefixContext *ctx) override;
  void exitBase_prefix(ModelicaParser::Base_prefixContext *ctx) override;
  void enterEnum_list(ModelicaParser::Enum_listContext *ctx) override;
  void exitEnum_list(ModelicaParser::Enum_listContext *ctx) override;
  void enterEnumeration_literal(ModelicaParser::Enumeration_literalContext *ctx) override;
  void exitEnumeration_literal(ModelicaParser::Enumeration_literalContext *ctx) override;
  void enterComposition(ModelicaParser::CompositionContext *ctx) override;
  void exitComposition(ModelicaParser::CompositionContext *ctx) override;
  void enterLanguage_specification(ModelicaParser::Language_specificationContext *ctx) override;
  void exitLanguage_specification(ModelicaParser::Language_specificationContext *ctx) override;
  void enterExternal_function_call(ModelicaParser::External_function_callContext *ctx) override;
  void exitExternal_function_call(ModelicaParser::External_function_callContext *ctx) override;
  void enterElement_list(ModelicaParser::Element_listContext *ctx) override;
  void exitElement_list(ModelicaParser::Element_listContext *ctx) override;
  void enterElement(ModelicaParser::ElementContext *ctx) override;
  void exitElement(ModelicaParser::ElementContext *ctx) override;
  void enterRegular_element(ModelicaParser::Regular_elementContext *ctx) override;
  void exitRegular_element(ModelicaParser::Regular_elementContext *ctx) override;
  void enterReplaceable_element(ModelicaParser::Replaceable_elementContext *ctx) override;
  void exitReplaceable_element(ModelicaParser::Replaceable_elementContext *ctx) override;
  void enterImport_clause(ModelicaParser::Import_clauseContext *ctx) override;
  void exitImport_clause(ModelicaParser::Import_clauseContext *ctx) override;
  void enterImport_list(ModelicaParser::Import_listContext *ctx) override;
  void exitImport_list(ModelicaParser::Import_listContext *ctx) override;
  void enterExtends_clause(ModelicaParser::Extends_clauseContext *ctx) override;
  void exitExtends_clause(ModelicaParser::Extends_clauseContext *ctx) override;
  void enterConstraining_clause(ModelicaParser::Constraining_clauseContext *ctx) override;
  void exitConstraining_clause(ModelicaParser::Constraining_clauseContext *ctx) override;
  void enterComponent_clause(ModelicaParser::Component_clauseContext *ctx) override;
  void exitComponent_clause(ModelicaParser::Component_clauseContext *ctx) override;
  void enterType_prefix(ModelicaParser::Type_prefixContext *ctx) override;
  void exitType_prefix(ModelicaParser::Type_prefixContext *ctx) override;
  void enterType_specifier_element(ModelicaParser::Type_specifier_elementContext *ctx) override;
  void exitType_specifier_element(ModelicaParser::Type_specifier_elementContext *ctx) override;
  void enterType_specifier(ModelicaParser::Type_specifierContext *ctx) override;
  void exitType_specifier(ModelicaParser::Type_specifierContext *ctx) override;
  void enterComponent_list(ModelicaParser::Component_listContext *ctx) override;
  void exitComponent_list(ModelicaParser::Component_listContext *ctx) override;
  void enterComponent_declaration(ModelicaParser::Component_declarationContext *ctx) override;
  void exitComponent_declaration(ModelicaParser::Component_declarationContext *ctx) override;
  void enterCondition_attribute(ModelicaParser::Condition_attributeContext *ctx) override;
  void exitCondition_attribute(ModelicaParser::Condition_attributeContext *ctx) override;
  void enterDeclaration(ModelicaParser::DeclarationContext *ctx) override;
  void exitDeclaration(ModelicaParser::DeclarationContext *ctx) override;
  void enterModification_class(ModelicaParser::Modification_classContext *ctx) override;
  void exitModification_class(ModelicaParser::Modification_classContext *ctx) override;
  void enterModification_assignment(ModelicaParser::Modification_assignmentContext *ctx) override;
  void exitModification_assignment(ModelicaParser::Modification_assignmentContext *ctx) override;
  void enterModification_assignment2(ModelicaParser::Modification_assignment2Context *ctx) override;
  void exitModification_assignment2(ModelicaParser::Modification_assignment2Context *ctx) override;
  void enterClass_modification(ModelicaParser::Class_modificationContext *ctx) override;
  void exitClass_modification(ModelicaParser::Class_modificationContext *ctx) override;
  void enterArgument_list(ModelicaParser::Argument_listContext *ctx) override;
  void exitArgument_list(ModelicaParser::Argument_listContext *ctx) override;
  void enterArgument(ModelicaParser::ArgumentContext *ctx) override;
  void exitArgument(ModelicaParser::ArgumentContext *ctx) override;
  void enterElement_modification_or_replaceable(ModelicaParser::Element_modification_or_replaceableContext *ctx) override;
  void exitElement_modification_or_replaceable(ModelicaParser::Element_modification_or_replaceableContext *ctx) override;
  void enterElement_modification(ModelicaParser::Element_modificationContext *ctx) override;
  void exitElement_modification(ModelicaParser::Element_modificationContext *ctx) override;
  void enterElement_redeclaration(ModelicaParser::Element_redeclarationContext *ctx) override;
  void exitElement_redeclaration(ModelicaParser::Element_redeclarationContext *ctx) override;
  void enterElement_replaceable(ModelicaParser::Element_replaceableContext *ctx) override;
  void exitElement_replaceable(ModelicaParser::Element_replaceableContext *ctx) override;
  void enterComponent_clause1(ModelicaParser::Component_clause1Context *ctx) override;
  void exitComponent_clause1(ModelicaParser::Component_clause1Context *ctx) override;
  void enterComponent_declaration1(ModelicaParser::Component_declaration1Context *ctx) override;
  void exitComponent_declaration1(ModelicaParser::Component_declaration1Context *ctx) override;
  void enterShort_class_definition(ModelicaParser::Short_class_definitionContext *ctx) override;
  void exitShort_class_definition(ModelicaParser::Short_class_definitionContext *ctx) override;
  void enterEquation_block(ModelicaParser::Equation_blockContext *ctx) override;
  void exitEquation_block(ModelicaParser::Equation_blockContext *ctx) override;
  void enterEquation_section(ModelicaParser::Equation_sectionContext *ctx) override;
  void exitEquation_section(ModelicaParser::Equation_sectionContext *ctx) override;
  void enterStatement_block(ModelicaParser::Statement_blockContext *ctx) override;
  void exitStatement_block(ModelicaParser::Statement_blockContext *ctx) override;
  void enterAlgorithm_section(ModelicaParser::Algorithm_sectionContext *ctx) override;
  void exitAlgorithm_section(ModelicaParser::Algorithm_sectionContext *ctx) override;
  void enterEquation_simple(ModelicaParser::Equation_simpleContext *ctx) override;
  void exitEquation_simple(ModelicaParser::Equation_simpleContext *ctx) override;
  void enterEquation_if(ModelicaParser::Equation_ifContext *ctx) override;
  void exitEquation_if(ModelicaParser::Equation_ifContext *ctx) override;
  void enterEquation_for(ModelicaParser::Equation_forContext *ctx) override;
  void exitEquation_for(ModelicaParser::Equation_forContext *ctx) override;
  void enterEquation_connect_clause(ModelicaParser::Equation_connect_clauseContext *ctx) override;
  void exitEquation_connect_clause(ModelicaParser::Equation_connect_clauseContext *ctx) override;
  void enterEquation_when(ModelicaParser::Equation_whenContext *ctx) override;
  void exitEquation_when(ModelicaParser::Equation_whenContext *ctx) override;
  void enterEquation_function(ModelicaParser::Equation_functionContext *ctx) override;
  void exitEquation_function(ModelicaParser::Equation_functionContext *ctx) override;
  void enterEquation(ModelicaParser::EquationContext *ctx) override;
  void exitEquation(ModelicaParser::EquationContext *ctx) override;
  void enterStatement_component_reference(ModelicaParser::Statement_component_referenceContext *ctx) override;
  void exitStatement_component_reference(ModelicaParser::Statement_component_referenceContext *ctx) override;
  void enterStatement_component_function(ModelicaParser::Statement_component_functionContext *ctx) override;
  void exitStatement_component_function(ModelicaParser::Statement_component_functionContext *ctx) override;
  void enterStatement_break(ModelicaParser::Statement_breakContext *ctx) override;
  void exitStatement_break(ModelicaParser::Statement_breakContext *ctx) override;
  void enterStatement_return(ModelicaParser::Statement_returnContext *ctx) override;
  void exitStatement_return(ModelicaParser::Statement_returnContext *ctx) override;
  void enterStatement_if(ModelicaParser::Statement_ifContext *ctx) override;
  void exitStatement_if(ModelicaParser::Statement_ifContext *ctx) override;
  void enterStatement_for(ModelicaParser::Statement_forContext *ctx) override;
  void exitStatement_for(ModelicaParser::Statement_forContext *ctx) override;
  void enterStatement_while(ModelicaParser::Statement_whileContext *ctx) override;
  void exitStatement_while(ModelicaParser::Statement_whileContext *ctx) override;
  void enterStatement_when(ModelicaParser::Statement_whenContext *ctx) override;
  void exitStatement_when(ModelicaParser::Statement_whenContext *ctx) override;
  void enterStatement(ModelicaParser::StatementContext *ctx) override;
  void exitStatement(ModelicaParser::StatementContext *ctx) override;
  void enterIf_equation(ModelicaParser::If_equationContext *ctx) override;
  void exitIf_equation(ModelicaParser::If_equationContext *ctx) override;
  void enterIf_statement(ModelicaParser::If_statementContext *ctx) override;
  void exitIf_statement(ModelicaParser::If_statementContext *ctx) override;
  void enterFor_equation(ModelicaParser::For_equationContext *ctx) override;
  void exitFor_equation(ModelicaParser::For_equationContext *ctx) override;
  void enterFor_statement(ModelicaParser::For_statementContext *ctx) override;
  void exitFor_statement(ModelicaParser::For_statementContext *ctx) override;
  void enterFor_indices(ModelicaParser::For_indicesContext *ctx) override;
  void exitFor_indices(ModelicaParser::For_indicesContext *ctx) override;
  void enterFor_index(ModelicaParser::For_indexContext *ctx) override;
  void exitFor_index(ModelicaParser::For_indexContext *ctx) override;
  void enterWhile_statement(ModelicaParser::While_statementContext *ctx) override;
  void exitWhile_statement(ModelicaParser::While_statementContext *ctx) override;
  void enterWhen_equation(ModelicaParser::When_equationContext *ctx) override;
  void exitWhen_equation(ModelicaParser::When_equationContext *ctx) override;
  void enterWhen_statement(ModelicaParser::When_statementContext *ctx) override;
  void exitWhen_statement(ModelicaParser::When_statementContext *ctx) override;
  void enterConnect_clause(ModelicaParser::Connect_clauseContext *ctx) override;
  void exitConnect_clause(ModelicaParser::Connect_clauseContext *ctx) override;
  void enterExpression_simple(ModelicaParser::Expression_simpleContext *ctx) override;
  void exitExpression_simple(ModelicaParser::Expression_simpleContext *ctx) override;
  void enterExpression_if(ModelicaParser::Expression_ifContext *ctx) override;
  void exitExpression_if(ModelicaParser::Expression_ifContext *ctx) override;
  void enterSimple_expression(ModelicaParser::Simple_expressionContext *ctx) override;
  void exitSimple_expression(ModelicaParser::Simple_expressionContext *ctx) override;
  void enterExpr_neg(ModelicaParser::Expr_negContext *ctx) override;
  void exitExpr_neg(ModelicaParser::Expr_negContext *ctx) override;
  void enterExpr_add(ModelicaParser::Expr_addContext *ctx) override;
  void exitExpr_add(ModelicaParser::Expr_addContext *ctx) override;
  void enterExpr_exp(ModelicaParser::Expr_expContext *ctx) override;
  void exitExpr_exp(ModelicaParser::Expr_expContext *ctx) override;
  void enterExpr_or(ModelicaParser::Expr_orContext *ctx) override;
  void exitExpr_or(ModelicaParser::Expr_orContext *ctx) override;
  void enterExpr_primary(ModelicaParser::Expr_primaryContext *ctx) override;
  void exitExpr_primary(ModelicaParser::Expr_primaryContext *ctx) override;
  void enterExpr_and(ModelicaParser::Expr_andContext *ctx) override;
  void exitExpr_and(ModelicaParser::Expr_andContext *ctx) override;
  void enterExpr_rel(ModelicaParser::Expr_relContext *ctx) override;
  void exitExpr_rel(ModelicaParser::Expr_relContext *ctx) override;
  void enterExpr_not(ModelicaParser::Expr_notContext *ctx) override;
  void exitExpr_not(ModelicaParser::Expr_notContext *ctx) override;
  void enterExpr_mul(ModelicaParser::Expr_mulContext *ctx) override;
  void exitExpr_mul(ModelicaParser::Expr_mulContext *ctx) override;
  void enterPrimary_unsigned_number(ModelicaParser::Primary_unsigned_numberContext *ctx) override;
  void exitPrimary_unsigned_number(ModelicaParser::Primary_unsigned_numberContext *ctx) override;
  void enterPrimary_string(ModelicaParser::Primary_stringContext *ctx) override;
  void exitPrimary_string(ModelicaParser::Primary_stringContext *ctx) override;
  void enterPrimary_false(ModelicaParser::Primary_falseContext *ctx) override;
  void exitPrimary_false(ModelicaParser::Primary_falseContext *ctx) override;
  void enterPrimary_true(ModelicaParser::Primary_trueContext *ctx) override;
  void exitPrimary_true(ModelicaParser::Primary_trueContext *ctx) override;
  void enterPrimary_function(ModelicaParser::Primary_functionContext *ctx) override;
  void exitPrimary_function(ModelicaParser::Primary_functionContext *ctx) override;
  void enterPrimary_derivative(ModelicaParser::Primary_derivativeContext *ctx) override;
  void exitPrimary_derivative(ModelicaParser::Primary_derivativeContext *ctx) override;
  void enterPrimary_initial(ModelicaParser::Primary_initialContext *ctx) override;
  void exitPrimary_initial(ModelicaParser::Primary_initialContext *ctx) override;
  void enterPrimary_component_reference(ModelicaParser::Primary_component_referenceContext *ctx) override;
  void exitPrimary_component_reference(ModelicaParser::Primary_component_referenceContext *ctx) override;
  void enterPrimary_output_expression_list(ModelicaParser::Primary_output_expression_listContext *ctx) override;
  void exitPrimary_output_expression_list(ModelicaParser::Primary_output_expression_listContext *ctx) override;
  void enterPrimary_expression_list(ModelicaParser::Primary_expression_listContext *ctx) override;
  void exitPrimary_expression_list(ModelicaParser::Primary_expression_listContext *ctx) override;
  void enterPrimary_function_arguments(ModelicaParser::Primary_function_argumentsContext *ctx) override;
  void exitPrimary_function_arguments(ModelicaParser::Primary_function_argumentsContext *ctx) override;
  void enterPrimary_end(ModelicaParser::Primary_endContext *ctx) override;
  void exitPrimary_end(ModelicaParser::Primary_endContext *ctx) override;
  void enterName(ModelicaParser::NameContext *ctx) override;
  void exitName(ModelicaParser::NameContext *ctx) override;
  void enterComponent_reference_element(ModelicaParser::Component_reference_elementContext *ctx) override;
  void exitComponent_reference_element(ModelicaParser::Component_reference_elementContext *ctx) override;
  void enterComponent_reference(ModelicaParser::Component_referenceContext *ctx) override;
  void exitComponent_reference(ModelicaParser::Component_referenceContext *ctx) override;
  void enterFunction_call_args(ModelicaParser::Function_call_argsContext *ctx) override;
  void exitFunction_call_args(ModelicaParser::Function_call_argsContext *ctx) override;
  void enterFunction_arguments(ModelicaParser::Function_argumentsContext *ctx) override;
  void exitFunction_arguments(ModelicaParser::Function_argumentsContext *ctx) override;
  void enterNamed_arguments(ModelicaParser::Named_argumentsContext *ctx) override;
  void exitNamed_arguments(ModelicaParser::Named_argumentsContext *ctx) override;
  void enterNamed_argument(ModelicaParser::Named_argumentContext *ctx) override;
  void exitNamed_argument(ModelicaParser::Named_argumentContext *ctx) override;
  void enterArgument_function(ModelicaParser::Argument_functionContext *ctx) override;
  void exitArgument_function(ModelicaParser::Argument_functionContext *ctx) override;
  void enterArgument_expression(ModelicaParser::Argument_expressionContext *ctx) override;
  void exitArgument_expression(ModelicaParser::Argument_expressionContext *ctx) override;
  void enterOutput_expression_list(ModelicaParser::Output_expression_listContext *ctx) override;
  void exitOutput_expression_list(ModelicaParser::Output_expression_listContext *ctx) override;
  void enterExpression_list(ModelicaParser::Expression_listContext *ctx) override;
  void exitExpression_list(ModelicaParser::Expression_listContext *ctx) override;
  void enterArray_subscripts(ModelicaParser::Array_subscriptsContext *ctx) override;
  void exitArray_subscripts(ModelicaParser::Array_subscriptsContext *ctx) override;
  void enterSubscript(ModelicaParser::SubscriptContext *ctx) override;
  void exitSubscript(ModelicaParser::SubscriptContext *ctx) override;
  void enterComment(ModelicaParser::CommentContext *ctx) override;
  void exitComment(ModelicaParser::CommentContext *ctx) override;
  void enterString_comment(ModelicaParser::String_commentContext *ctx) override;
  void exitString_comment(ModelicaParser::String_commentContext *ctx) override;
  void enterAnnotation(ModelicaParser::AnnotationContext *ctx) override;
  void exitAnnotation(ModelicaParser::AnnotationContext *ctx) override;
};

} // cymoca

#endif //CYMOCA_COMPILER_H
