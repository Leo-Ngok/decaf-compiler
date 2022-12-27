#include "ast/ast.hpp"
#include "ast/visitor.hpp"
#include "config.hpp"

using namespace mind;
using namespace mind::ast;

ArrayRef::ArrayRef(std::string var_name, ExprList* rankList, Location *l){
    this->var = var_name;
    this->ranklist = rankList;
    setBasicInfo(ARRAY_REF, l);
}

void ArrayRef::accept(Visitor *v) { v->visit(this); }
void ArrayRef::dumpTo(std::ostream &os) {
    ASTNode::dumpTo(os);
    os << ranklist;
    os << ")";
    decIndent(os);
}

