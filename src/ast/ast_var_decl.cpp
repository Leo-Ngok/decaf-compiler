/*****************************************************
 *  Implementation of "VarDecl".
 *
 *  Please refer to ast/ast.hpp for the definition.
 *
 *  Keltin Leung 
 */

#include "ast/ast.hpp"
#include "ast/visitor.hpp"
#include "config.hpp"

using namespace mind;
using namespace mind::ast;

/* Creates a new VarDecl node.
 *
 * PARAMETERS:
 *   n       - name of the variable
 *   t       - type of the variable
 *   l       - position in the source text
 */
VarDecl::VarDecl(std::string n, Type *t, Location *l) {

    setBasicInfo(VAR_DECL, l);

    name = n;
    type = t;
    init = NULL;
}

VarDecl::VarDecl(std::string n, Type *t, Expr *i, Location *l) {
    setBasicInfo(VAR_DECL, l);

    name = n;
    type = t;
    init = i;
}

VarDecl::VarDecl(std::string n, Type *t, int d, Location *l) {

    setBasicInfo(VAR_DECL, l);

    name = n;
    type = t;
    init = NULL;
    initList = nullptr;
}


VarDecl::VarDecl(std::string n, Type* t, DimList *init_list, Location *l){
    setBasicInfo(VAR_DECL, l);
    name = n;
    type = t;
    initList = init_list;
    init = NULL;
}

/* Visits the current node.
 *
 * PARAMETERS:
 *   v       - the visitor
 */
void VarDecl::accept(Visitor *v) { v->visit(this); }

/* Prints the current AST node.
 *
 * PARAMETERS:
 *   os      - the output stream
 */
void VarDecl::dumpTo(std::ostream &os) {
    ASTNode::dumpTo(os);
    if (init != NULL) {
        os << " " << '"' << name << '"' << " " << type << ")";
        newLine(os);
        os << init << ")";
    } else if (initList != NULL){
        os << " " << '"' << name << '"' << " " << type << ")";
        newLine(os);
        os << initList << ")";
    } else {
        os << " " << '"' << name << '"' << " " << type << "=";
        
    }
    decIndent(os);
}
