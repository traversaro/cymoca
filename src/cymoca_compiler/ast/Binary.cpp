//
// Created by jgoppert on 6/28/18.
//

#include "Binary.h"

namespace cymoca {
namespace ast {

unordered_map<BinaryOp, string, EnumClassHash> binaryOpStr = {
    {BinaryOp::ADD, "+"},   {BinaryOp::SUB, "-"},  {BinaryOp::MUL, "*"},
    {BinaryOp::DIV, "/"},   {BinaryOp::POW, "^"},  {BinaryOp::EMUL, ".*"},
    {BinaryOp::EDIV, "./"}, {BinaryOp::EPOW, ".^"}};

unordered_map<RelationOp, string, EnumClassHash> relationOpStr = {
    {RelationOp::LT, "<"},  {RelationOp::LE, "<="}, {RelationOp::GT, ">"},
    {RelationOp::GE, ">="}, {RelationOp::EQ, "="},  {RelationOp::NEQ, "<>"}};

unordered_map<BinaryLogicOp, string, EnumClassHash> binaryLogicOpStr = {
    {BinaryLogicOp::AND, "and"},
    {BinaryLogicOp::OR, "or"},
};

} // namespace ast
} // namespace cymoca

// vim: set et fenc=utf-8 ff=unix sts=0 sw=2 ts=2 :
