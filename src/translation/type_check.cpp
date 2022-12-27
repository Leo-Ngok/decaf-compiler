/*****************************************************
 *  Implementation of the second semantic analysis pass.
 *
 *  In the second pass, we will check:
 *    1. whether all the expressions are well-typed; (and sets ATTR(type))
 *    2. whether all the statements are well-formed;
 *    3. whether all the referenced symbols are well-defined. (and sets
 * ATTR(sym))
 *
 *  Keltin Leung 
 */

#include "3rdparty/list.hpp"
#include "ast/ast.hpp"
#include "ast/visitor.hpp"
#include "compiler.hpp"
#include "config.hpp"
#include "scope/scope_stack.hpp"
#include "symb/symbol.hpp"
#include "type/type.hpp"

using namespace mind;
using namespace mind::type;
using namespace mind::scope;
using namespace mind::symb;
using namespace mind::util;
using namespace mind::err;

/* Pass 2 of the semantic analysis.
 */
class SemPass2 : public ast::Visitor {
    // Visiting expressions
    virtual void visit(ast::AssignExpr *);
    virtual void visit(ast::AddExpr *);
    virtual void visit(ast::SubExpr *);
    virtual void visit(ast::MulExpr *);
    virtual void visit(ast::DivExpr *);
    virtual void visit(ast::ModExpr *);
    virtual void visit(ast::EquExpr *);
    virtual void visit(ast::NeqExpr *);
    virtual void visit(ast::LesExpr *);
    virtual void visit(ast::GrtExpr *);
    virtual void visit(ast::LeqExpr *);
    virtual void visit(ast::GeqExpr *);
    virtual void visit(ast::AndExpr *);
    virtual void visit(ast::OrExpr *);
    virtual void visit(ast::IntConst *);
    virtual void visit(ast::NegExpr *);
    virtual void visit(ast::NotExpr *);
    virtual void visit(ast::BitNotExpr *);
    virtual void visit(ast::LvalueExpr *);
    virtual void visit(ast::VarRef *);
    virtual void visit(ast::IfExpr *);
    virtual void visit(ast::FuncRef *);
    virtual void visit(ast::ArrayRef *);
    // Visiting statements
    virtual void visit(ast::VarDecl *);
    virtual void visit(ast::CompStmt *);
    virtual void visit(ast::ExprStmt *);
    virtual void visit(ast::IfStmt *);
    virtual void visit(ast::ReturnStmt *);
    virtual void visit(ast::WhileStmt *);
    // Visiting declarations
    virtual void visit(ast::FuncDefn *);
    virtual void visit(ast::Program *);
};

// recording the current return type
static Type *retType = NULL;
// recording the current "this" type

/* Determines whether a given type is BaseType::Error.
 *
 * NOTE:
 *   don't use the == operator when comparing types
 * PARAMETERS:
 *   t     - the type to check
 */
static bool isErrorType(Type *t) { return t->equal(BaseType::Error); }

/* Checks whether an ast::Expr conforms to the expecting type.
 *
 * NOTE:
 *   if the expression type is BaseType::Error, we accept it as a legal case.
 * PARAMETERS:
 *   e     - the ast::Expr node
 *   t     - the expected type
 * SIDE-EFFECTS:
 *   Unexpected Type Error may be issued
 */
static void expect(ast::Expr *e, Type *t) {
    if (!e->ATTR(type)->equal(t) && !isErrorType(e->ATTR(type))) {
        issue(e->getLocation(), new UnexpectedTypeError(t, e->ATTR(type)));
    }
}

/* Visits an ast::IntConst node.
 *
 * PARAMETERS:
 *   e     - the ast::IntConst node
 */
void SemPass2::visit(ast::IntConst *e) { e->ATTR(type) = BaseType::Int; }

/* Visits an ast::AddExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::AddExpr node
 */
void SemPass2::visit(ast::AddExpr *e) {
    e->e1->accept(this);
    expect(e->e1, BaseType::Int);

    e->e2->accept(this);
    expect(e->e2, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::SubExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::SubExpr node
 */
void SemPass2::visit(ast::SubExpr *e) {
    e->e1->accept(this);
    expect(e->e1, BaseType::Int);

    e->e2->accept(this);
    expect(e->e2, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::MulExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::MulExpr node
 */
void SemPass2::visit(ast::MulExpr *e) {
    e->e1->accept(this);
    expect(e->e1, BaseType::Int);

    e->e2->accept(this);
    expect(e->e2, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::DivExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::DivExpr node
 */
void SemPass2::visit(ast::DivExpr *e) {
    e->e1->accept(this);
    expect(e->e1, BaseType::Int);

    e->e2->accept(this);
    expect(e->e2, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::ModExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::ModExpr node
 */
void SemPass2::visit(ast::ModExpr *e) {
    e->e1->accept(this);
    expect(e->e1, BaseType::Int);

    e->e2->accept(this);
    expect(e->e2, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::EquExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::EquExpr node
 */
void SemPass2::visit(ast::EquExpr *e) {
    e->e1->accept(this);
    expect(e->e1, BaseType::Int);

    e->e2->accept(this);
    expect(e->e2, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::NeqExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::NeqExpr node
 */
void SemPass2::visit(ast::NeqExpr *e) {
    e->e1->accept(this);
    expect(e->e1, BaseType::Int);

    e->e2->accept(this);
    expect(e->e2, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::LesExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::LesExpr node
 */
void SemPass2::visit(ast::LesExpr *e) {
    e->e1->accept(this);
    expect(e->e1, BaseType::Int);

    e->e2->accept(this);
    expect(e->e2, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::GrtExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::GrtExpr node
 */
void SemPass2::visit(ast::GrtExpr *e) {
    e->e1->accept(this);
    expect(e->e1, BaseType::Int);

    e->e2->accept(this);
    expect(e->e2, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::LeqExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::LeqExpr node
 */
void SemPass2::visit(ast::LeqExpr *e) {
    e->e1->accept(this);
    expect(e->e1, BaseType::Int);

    e->e2->accept(this);
    expect(e->e2, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::GeqExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::GeqExpr node
 */
void SemPass2::visit(ast::GeqExpr *e) {
    e->e1->accept(this);
    expect(e->e1, BaseType::Int);

    e->e2->accept(this);
    expect(e->e2, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::AndExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::AndExpr node
 */
void SemPass2::visit(ast::AndExpr *e) {
    e->e1->accept(this);
    expect(e->e1, BaseType::Int);

    e->e2->accept(this);
    expect(e->e2, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::OrExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::OrExpr node
 */
void SemPass2::visit(ast::OrExpr *e) {
    e->e1->accept(this);
    expect(e->e1, BaseType::Int);

    e->e2->accept(this);
    expect(e->e2, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::NegExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::NegExpr node
 */
void SemPass2::visit(ast::NegExpr *e) {
    e->e->accept(this);
    expect(e->e, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::NotExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::NotExpr node
 */
void SemPass2::visit(ast::NotExpr *e) {
    e->e->accept(this);
    expect(e->e, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::BitNotExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::BitNotExpr node
 */
void SemPass2::visit(ast::BitNotExpr *e) {
    e->e->accept(this);
    expect(e->e, BaseType::Int);

    e->ATTR(type) = BaseType::Int;
}

/* Visits an ast::LvalueExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::LvalueExpr node
 */
void SemPass2::visit(ast::LvalueExpr *e) {
    e->lvalue->accept(this);
    e->ATTR(type) = e->lvalue->ATTR(type);
}

/* Visits an ast::VarRef node.
 *
 * PARAMETERS:
 *   e     - the ast::VarRef node
 */
void SemPass2::visit(ast::VarRef *ref) {
    // CASE I: owner is NULL ==> referencing a local var or a member var?
    Symbol *v = scopes->lookup(ref->var, ref->getLocation());
    if (NULL == v) {
        issue(ref->getLocation(), new SymbolNotFoundError(ref->var));
        goto issue_error_type;

    } else if (!v->isVariable()) {
        issue(ref->getLocation(), new NotVariableError(v));
        goto issue_error_type;

    } //else if(!v->getType()->isBaseType()) {
        //issue(ref->getLocation(), new NotVariableError(v));
        //goto issue_error_type;
    //} 
    else {
        if(v->getType()->isBaseType())
            ref->ATTR(type) = v->getType();
        else if(v->getType()->isArrayType())
            ref->ATTR(type) = v->getType();
        else 
            mind_assert(false);
        ref->ATTR(sym) = (Variable *)v;

        if (((Variable *)v)->isLocalVar()) {
            if(v->getType()->isBaseType())
            ref->ATTR(lv_kind) = ast::Lvalue::SIMPLE_VAR;
            //else if(v->getType()->isArrayType())
            //ref->ATTR(lv_kind) = ast::Lvalue::ARRAY_ELE;
        }
    }

    return;

    // sometimes "GOTO" will make things simpler. this is one of such cases:
issue_error_type:
    ref->ATTR(type) = BaseType::Error;
    ref->ATTR(sym) = NULL;
    return;
}

void SemPass2::visit(ast::ArrayRef *aref){
    Symbol *v = scopes->lookup(aref->var, aref->getLocation());
    if (NULL == v) {
        issue(aref->getLocation(), new SymbolNotFoundError(aref->var));
        goto issue_error_type;

    } else if (!v->isVariable()) {
        issue(aref->getLocation(), new NotVariableError(v));
        goto issue_error_type;

    } else if(!v->getType()->isArrayType()) {
        issue(aref->getLocation(), new NotArrayError());
        goto issue_error_type;
    } else {
        
        aref->ATTR(sym) = (Variable *)v;
        ArrayType* atype = (ArrayType*)v->getType();
        if(atype->getLevel() != aref->ranklist->length()) {
            issue(aref->getLocation(), new NotArrayError());
        }
        else {
            Type* curr_type = atype;
            for(auto it = aref->ranklist->begin();
            it != aref->ranklist->end(); ++it) {
                (*it)->accept(this);
                if(!(*it)->ATTR(type)->compatible(BaseType::Int)) {
                    issue((*it)->getLocation(), new UnexpectedTypeError((*it)->ATTR(type), BaseType::Int));
                }
                curr_type = ((ArrayType*)curr_type)->getElementType();
            }
            aref->ATTR(type) = curr_type;
        }
        //if (((Variable *)v)->isLocalVar()) {
            aref->ATTR(lv_kind) = ast::Lvalue::ARRAY_ELE;
        //}
    }

    return;

    // sometimes "GOTO" will make things simpler. this is one of such cases:
issue_error_type:
    aref->ATTR(type) = BaseType::Error;
    aref->ATTR(sym) = NULL;
    return;
}

/* Visits an ast::VarDecl node.
 *
 * PARAMETERS:
 *   decl     - the ast::VarDecl node
 */
void SemPass2::visit(ast::VarDecl *decl) {
    if (decl->init)
        decl->init->accept(this);
}

/* Visits an ast::AssignStmt node.
 *
 * PARAMETERS:
 *   e     - the ast::AssignStmt node
 */
void SemPass2::visit(ast::AssignExpr *s) {
    s->left->accept(this);
    s->e->accept(this);

    if (!isErrorType(s->left->ATTR(type)) &&
        !s->e->ATTR(type)->compatible(s->left->ATTR(type))) {
        issue(s->getLocation(),
              new IncompatibleError(s->left->ATTR(type), s->e->ATTR(type)));
    }

    s->ATTR(type) = s->left->ATTR(type);
}

/* Visits an ast::IfExpr node.
 *
 * PARAMETERS:
 *   e     - the ast::IfExpr node
 */
void SemPass2::visit(ast::IfExpr *s) {
    s->condition->accept(this);
    if(!s->condition->ATTR(type)->equal(BaseType::Int)) {
        issue(s->condition->getLocation(), new BadTestExprError());
    }
    s->true_brch->accept(this);
    s->false_brch->accept(this);
    // TODO: (optional) check if the true/false branches
    // have consistent types (as it is assignment).
    if(s->true_brch->ATTR(type) == s->false_brch->ATTR(type)) {
        s->ATTR(type) = s->true_brch->ATTR(type);
    } else {
        // TODO: yield false assertion
        // Fine if it is not assignment statement
        // However, it is not checkable here.
    }
}

/* Visits an ast::ExprStmt node.
 *
 * PARAMETERS:
 *   e     - the ast::ExprStmt node
 */
void SemPass2::visit(ast::ExprStmt *s) { s->e->accept(this); }

/* Visits an ast::IfStmt node.
 *
 * PARAMETERS:
 *   e     - the ast::IfStmt node
 */
void SemPass2::visit(ast::IfStmt *s) {
    s->condition->accept(this);
    if (!s->condition->ATTR(type)->equal(BaseType::Int)) {
        issue(s->condition->getLocation(), new BadTestExprError());
        ;
    }

    s->true_brch->accept(this);
    s->false_brch->accept(this);
}

/* Visits an ast::CompStmt node.
 *
 * PARAMETERS:
 *   c     - the ast::CompStmt node
 */
void SemPass2::visit(ast::CompStmt *c) {
    scopes->open(c->ATTR(scope));
    for (auto it = c->stmts->begin(); it != c->stmts->end(); ++it)
        (*it)->accept(this);
    scopes->close();
}
/* Visits an ast::WhileStmt node.
 *
 * PARAMETERS:
 *   e     - the ast::WhileStmt node
 */
void SemPass2::visit(ast::WhileStmt *s) {
    s->condition->accept(this);
    if (!s->condition->ATTR(type)->equal(BaseType::Int)) {
        issue(s->condition->getLocation(), new BadTestExprError());
    }

    s->loop_body->accept(this);
}

/* Visits an ast::ReturnStmt node.
 *
 * PARAMETERS:
 *   e     - the ast::ReturnStmt node
 */
void SemPass2::visit(ast::ReturnStmt *s) {
    s->e->accept(this);

    if (!isErrorType(retType) && !s->e->ATTR(type)->compatible(retType)) {
        issue(s->e->getLocation(),
              new IncompatibleError(retType, s->e->ATTR(type)));
    }
}

/* Visits an ast::FunDefn node.
 *
 * PARAMETERS:
 *   e     - the ast::FunDefn node
 */
void SemPass2::visit(ast::FuncDefn *f) {
    ast::StmtList::iterator it;

    retType = f->ret_type->ATTR(type);

    scopes->open(f->ATTR(sym)->getAssociatedScope());
    for (it = f->stmts->begin(); it != f->stmts->end(); ++it)
        (*it)->accept(this);
    scopes->close();
}


void SemPass2::visit(ast::FuncRef *ref) {
    // CASE I: owner is NULL ==> referencing a local var or a member var?
    Symbol *v = scopes->lookup(ref->name, ref->getLocation());
    if (NULL == v) {
        issue(ref->getLocation(), new SymbolNotFoundError(ref->name));
        goto issue_error_type;

    } else if (!v->isFunction()) {
        issue(ref->getLocation(), new NotMethodError(v));
        goto issue_error_type;

    } else goto success;
        

    // sometimes "GOTO" will make things simpler. this is one of such cases:
issue_error_type:
    ref->ATTR(type) = BaseType::Error;
    ref->ATTR(sym) = NULL;
    return;
success:
    Function * __fv = dynamic_cast<Function *>(v);
    auto formal_args = __fv->getType()->getArgList();
    auto actual_args = ref->args;
    if(formal_args->length() != actual_args->length()) {
        issue(ref->getLocation(), new BadArgCountError(__fv));
        goto issue_error_type;
    }
    
    auto fargit = formal_args->begin();
    auto aargit = actual_args->begin();
    while(fargit != formal_args->end()) {
        (*aargit)->accept(this);
        if(!(*fargit)->compatible(((*aargit)->ATTR(type))) &&
        !((*fargit)->isArrayType() && (*aargit)->ATTR(type)->isArrayType())) {
            issue((*aargit)->getLocation(), new UnexpectedTypeError(
                (*aargit)->ATTR(type), *fargit
            ));
            goto issue_error_type;
        }
        ++fargit;
        ++aargit;
    }
    ref->ATTR(type) =  __fv->getResultType();
    ref->ATTR(sym) = (Function*) v;
    return;
}

/* Visits an ast::Program node.
 *
 * PARAMETERS:
 *   e     - the ast::Program node
 */
void SemPass2::visit(ast::Program *p) {
    scopes->open(p->ATTR(gscope));
    for (auto it = p->func_and_globals->begin();
         it != p->func_and_globals->end(); ++it)
        (*it)->accept(this);
    scopes->close(); // close the global scope
}

/* Checks the types of all the expressions.
 *
 * PARAMETERS:
 *   tree  - AST of the program
 */
void MindCompiler::checkTypes(ast::Program *tree) {
    tree->accept(new SemPass2());
}
