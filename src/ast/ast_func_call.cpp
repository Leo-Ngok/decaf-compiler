#include "ast/ast.hpp"
#include "ast/visitor.hpp"
#include "config.hpp"

using namespace mind;
using namespace mind::ast;

FuncRef::FuncRef(std::string __name, ExprList * __args, Location* l) {
    setBasicInfo(FUNC_REF, l);
    name = __name;
    args = __args;
}
void FuncRef::accept(Visitor *v){
    v->visit(this);
}

void FuncRef::dumpTo(std::ostream &os) {
    ASTNode::dumpTo(os);
    newLine(os);
    os << name;
    newLine(os);
    os << args << ")";
    decIndent(os);
}