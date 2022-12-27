#include "ast/ast.hpp"
#include "ast/visitor.hpp"
#include "config.hpp"

using namespace mind;
using namespace mind::ast;


ArrayType::ArrayType(Type* bt, DimList* __dimList, Location *l){
    base_type = bt;
    this->dimList = __dimList;
    setBasicInfo(ARRAY_TYPE, l);
}
    
void ArrayType::accept(Visitor *v) {
    v->visit(this);
}
void ArrayType::dumpTo(std::ostream &os) {
    ASTNode::dumpTo(os);
    os << dimList;
    os << ")";
    decIndent(os);
}
