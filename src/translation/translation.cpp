/*****************************************************
 *  Implementation of the third translation pass.
 *
 *  In the third pass, we will:
 *    translate all the statements and expressions
 *
 *  Keltin Leung 
 */

#include "translation.hpp"
#include "asm/offset_counter.hpp"
#include "ast/ast.hpp"
#include "compiler.hpp"
#include "config.hpp"
#include "scope/scope.hpp"
#include "symb/symbol.hpp"
#include "tac/tac.hpp"
#include "tac/trans_helper.hpp"
#include "type/type.hpp"

using namespace mind;
using namespace mind::symb;
using namespace mind::tac;
using namespace mind::type;
using namespace mind::assembly;

/* Constructor.
 *
 * PARAMETERS:
 *   helper - the translation helper
 */
Translation::Translation(tac::TransHelper *helper) {
    mind_assert(NULL != helper);

    tr = helper;
}

/* Translating an ast::Program node.
 */
void Translation::visit(ast::Program *p) {
    
    for (auto it = p->func_and_globals->begin();
         it != p->func_and_globals->end(); ++it)
         if((*it)->getKind() == ast::ASTNode::FUNC_DEFN) {
            (*it)->accept(this);
         }
    for (auto it = p->func_and_globals->begin();
         it != p->func_and_globals->end(); ++it)
         if((*it)->getKind() == ast::ASTNode::VAR_DECL) {
            (*it)->accept(this);
         }
}

// three sugars for parameter offset management
#define RESET_OFFSET() tr->getOffsetCounter()->reset(OffsetCounter::PARAMETER)
#define NEXT_OFFSET(x) tr->getOffsetCounter()->next(OffsetCounter::PARAMETER, x)

/* Translating an ast::FuncDefn node.
 *
 * NOTE:
 *   call tr->startFunc() before translating the statements and
 *   call tr->endFunc() after all the statements have been translated
 */
void Translation::visit(ast::FuncDefn *f) {
    Function *fun = f->ATTR(sym);
    // attaching function entry label
    fun->attachEntryLabel(tr->getNewEntryLabel(fun));
    
    if(f->forward_decl)
        return;
    // arguments
    int order = 0;
    for (auto it = f->formals->begin(); it != f->formals->end(); ++it) {
        auto v = (*it)->ATTR(sym);
        v->setOrder(order++);
        v->attachTemp(tr->getNewTempI4());
    }

    fun->offset = fun->getOrder() * POINTER_SIZE;

    RESET_OFFSET();

    tr->startFunc(fun);
    
    // You may process params here, i.e use reg or stack to pass parameters
    for (auto it = f->formals->begin(); it != f->formals->end(); ++it) {
        auto v = (*it)->ATTR(sym);
        tr->genFetchArg(v->getTemp(), v->getOrder());
    }


    // translates statement by statement
    for (auto it = f->stmts->begin(); it != f->stmts->end(); ++it)
        (*it)->accept(this);

    tr->genReturn(tr->genLoadImm4(0)); // Return 0 by default

    tr->endFunc();
}
void Translation::visit(ast::FuncRef *s) {
    for(auto it = s->args->begin(); it != s->args->end(); ++it) {
        (*it)->accept(this);
    }
    int total_args = s->args->length();
    int j = total_args-1;
    for(auto rit = s->args->rbegin(); rit != s->args->rend(); ++rit,--j) {
            tr->genSaveArg(j, (*rit)->ATTR(val));
    }
    s->ATTR(val) = tr->genCall(s->ATTR(sym)->getEntryLabel());
}
/* Translating an ast::AssignStmt node.
 *
 * NOTE:
 *   different kinds of Lvalue require different translation
 */
void Translation::visit(ast::AssignExpr *s) {
    // TODO
    s->e->accept(this);
    s->left->accept(this);
    s->ATTR(val) = s->e->ATTR(val);//lvar->ATTR(sym)->getTemp();
    switch(s->left->ATTR(lv_kind)){
    case s->left->SIMPLE_VAR: {
        ast::VarRef* lvar = (ast::VarRef*) s->left;
        if(lvar->ATTR(sym)->isGlobalVar()) {
            tr->genSaveGSym(lvar->ATTR(sym),s->e->ATTR(val));
        } else {
            tr->genAssign(lvar->ATTR(sym)->getTemp(), s->e->ATTR(val));
        }
        
        break;
    }
    default: break;
    }
    
}

/* Translating an ast::ExprStmt node.
 */
void Translation::visit(ast::ExprStmt *s) { s->e->accept(this); }

/* Translating an ast::IfStmt node.
 *
 * NOTE:
 *   you don't need to test whether the false_brch is empty
 */
void Translation::visit(ast::IfStmt *s) {
    Label L1 = tr->getNewLabel(); // entry of the false branch
    Label L2 = tr->getNewLabel(); // exit
    s->condition->accept(this);
    tr->genJumpOnZero(L1, s->condition->ATTR(val));

    s->true_brch->accept(this);
    tr->genJump(L2); // done

    tr->genMarkLabel(L1);
    s->false_brch->accept(this);

    tr->genMarkLabel(L2);
}
/* Translating an ast::WhileStmt node.
 */
void Translation::visit(ast::WhileStmt *s) {
    Label L1 = tr->getNewLabel();
    Label L2 = tr->getNewLabel();
    Label L3 = tr->getNewLabel();
    Label old_break = current_break_label;
    current_break_label = L2;
    Label legacy_continue = current_continue_label;
    current_continue_label = L1;
    tr->genMarkLabel(L1);
    if(s->condition != nullptr)
        s->condition->accept(this);
    tr->genJumpOnZero(L2, s->condition->ATTR(val));
    if(s->is_for){
        current_continue_label = L3;
        auto stmtsptr = dynamic_cast<ast::CompStmt*>(s->loop_body)->stmts->begin();
        /* Loop body */
        (*stmtsptr)->accept(this);
        stmtsptr++;
        /* Insert continue label */
        tr->genMarkLabel(L3);
        /* step expression */
        if(stmtsptr != dynamic_cast<ast::CompStmt*>(s->loop_body)->stmts->end())
            (*stmtsptr)->accept(this);
        tr->genJump(L1);
    } else {
        s->loop_body->accept(this);
        tr->genJump(L1);
    }

    tr->genMarkLabel(L2);
    current_continue_label = legacy_continue;
    current_break_label = old_break;
}

/* Translating an ast::BreakStmt node.
 */
void Translation::visit(ast::BreakStmt *s) { tr->genJump(current_break_label); }


/* Translating an ast::ContStmt node.
*/
void Translation::visit(ast::ContStmt *s) {
    tr->genJump(current_continue_label);
}
/* Translating an ast::CompStmt node.
 */
void Translation::visit(ast::CompStmt *c) {
    // translates statement by statement
    for (auto it = c->stmts->begin(); it != c->stmts->end(); ++it)
        (*it)->accept(this);
}
/* Translating an ast::ReturnStmt node.
 */
void Translation::visit(ast::ReturnStmt *s) {
    s->e->accept(this);
    tr->genReturn(s->e->ATTR(val));
}

/* Translating an ast::AddExpr node.
 */
void Translation::visit(ast::AddExpr *e) {
    e->e1->accept(this);
    e->e2->accept(this);

    e->ATTR(val) = tr->genAdd(e->e1->ATTR(val), e->e2->ATTR(val));
}

/* Translating an ast::SubExpr node.
 */
void Translation::visit(ast::SubExpr *e) {
    e->e1->accept(this);
    e->e2->accept(this);

    e->ATTR(val) = tr->genSub(e->e1->ATTR(val), e->e2->ATTR(val));
}

/* Translating an ast::MulExpr node.
 */
void Translation::visit(ast::MulExpr *e) {
    e->e1->accept(this);
    e->e2->accept(this);

    e->ATTR(val) = tr->genMul(e->e1->ATTR(val), e->e2->ATTR(val));
}

/* Translating an ast::DivExpr node.
 */
void Translation::visit(ast::DivExpr *e) {
    e->e1->accept(this);
    e->e2->accept(this);

    e->ATTR(val) = tr->genDiv(e->e1->ATTR(val), e->e2->ATTR(val));
}

/* Translating an ast::ModExpr node.
 */
void Translation::visit(ast::ModExpr *e) {
    e->e1->accept(this);
    e->e2->accept(this);

    e->ATTR(val) = tr->genMod(e->e1->ATTR(val), e->e2->ATTR(val));
}

/* Translating an ast::AddExpr node.
 */
void Translation::visit(ast::EquExpr *e) {
    e->e1->accept(this);
    e->e2->accept(this);

    e->ATTR(val) = tr->genEqu(e->e1->ATTR(val), e->e2->ATTR(val));
}

/* Translating an ast::NeqExpr node.
 */
void Translation::visit(ast::NeqExpr *e) {
    e->e1->accept(this);
    e->e2->accept(this);

    e->ATTR(val) = tr->genNeq(e->e1->ATTR(val), e->e2->ATTR(val));
}

/* Translating an ast::LesExpr node.
 */
void Translation::visit(ast::LesExpr *e) {
    e->e1->accept(this);
    e->e2->accept(this);

    e->ATTR(val) = tr->genLes(e->e1->ATTR(val), e->e2->ATTR(val));
}

/* Translating an ast::GrtExpr node.
 */
void Translation::visit(ast::GrtExpr *e) {
    e->e1->accept(this);
    e->e2->accept(this);

    e->ATTR(val) = tr->genGtr(e->e1->ATTR(val), e->e2->ATTR(val));
}

/* Translating an ast::LeqExpr node.
 */
void Translation::visit(ast::LeqExpr *e) {
    e->e1->accept(this);
    e->e2->accept(this);

    e->ATTR(val) = tr->genLeq(e->e1->ATTR(val), e->e2->ATTR(val));
}

/* Translating an ast::GeqExpr node.
 */
void Translation::visit(ast::GeqExpr *e) {
    e->e1->accept(this);
    e->e2->accept(this);

    e->ATTR(val) = tr->genGeq(e->e1->ATTR(val), e->e2->ATTR(val));
}

/* Translating an ast::AndExpr node.
 */
void Translation::visit(ast::AndExpr *e) {
    e->e1->accept(this);
    e->e2->accept(this);

    e->ATTR(val) = tr->genLAnd(e->e1->ATTR(val), e->e2->ATTR(val));
}

/* Translating an ast::OrExpr node.
 */
void Translation::visit(ast::OrExpr *e) {
    e->e1->accept(this);
    e->e2->accept(this);

    e->ATTR(val) = tr->genLOr(e->e1->ATTR(val), e->e2->ATTR(val));
}

/* Translating an ast::IntConst node.
 */
void Translation::visit(ast::IntConst *e) {
    e->ATTR(val) = tr->genLoadImm4(e->value);
}

/* Translating an ast::NegExpr node.
 */
void Translation::visit(ast::NegExpr *e) {
    e->e->accept(this);

    e->ATTR(val) = tr->genNeg(e->e->ATTR(val));
}

/* Translating an ast::NotExpr node.
 */
void Translation::visit(ast::NotExpr *e) {
    e->e->accept(this);

    e->ATTR(val) = tr->genLNot(e->e->ATTR(val));
}

/* Translating an ast::BitNotExpr node.
 */
void Translation::visit(ast::BitNotExpr *e) {
    e->e->accept(this);

    e->ATTR(val) = tr->genBNot(e->e->ATTR(val));
}

/* Translating an ast::LvalueExpr node.
 *
 * NOTE:
 *   different Lvalue kinds need different translation
 */
void Translation::visit(ast::LvalueExpr *e) {
    e->lvalue->accept(this);
    switch (e->lvalue->ATTR(lv_kind))
    {
    case e->lvalue->SIMPLE_VAR:
        if(( (ast::VarRef *) e->lvalue)->ATTR(sym)->isGlobalVar()) {
        e->ATTR(val) = tr->genLoadGSym(((ast::VarRef *) e->lvalue)->ATTR(sym));
        } else
        e->ATTR(val) = ( (ast::VarRef *) e->lvalue)->ATTR(sym)->getTemp();
        break;
    
    default:
        break;
    }
    
}

/* Translating an ast::VarRef node.
 *
 * NOTE:
 *   there are two kinds of variable reference: member variables or simple
 * variables
 */
void Translation::visit(ast::VarRef *ref) {
    switch (ref->ATTR(lv_kind)) {
    case ast::Lvalue::SIMPLE_VAR:
        // nothing to do
        break;

    default:
        mind_assert(false); // impossible
    }
    // actually it is so simple :-)
}

/* Translating an ast::VarDecl node.
 */
void Translation::visit(ast::VarDecl *decl) {
    if(decl->ATTR(sym)->isGlobalVar()) {
        tr->genGlobl(decl->ATTR(sym));
        return;
    }
    Temp var = tr->getNewTempI4();
    decl->ATTR(sym)->attachTemp(var);
    if(decl->init != NULL) {
        decl->init->accept(this);
        tr->genAssign(var, decl->init->ATTR(val));
    } else {
        Temp default_init = tr->genLoadImm4(0);
        tr->genAssign(var, default_init);
    }

    // TODO
}

/* Translating an ast::IfExpr node.
 */
void Translation::visit(ast::IfExpr *e) {
    // Note: this statement is equivalent to the following:

    // T ret;
    // if(CONDITION)
    //      ret = TRUE_STMT
    // else 
    //      ret = FALSE_STMT

    // Its 8086 assembly would be in this format
    // ret is assigned to eax

    // cmp CONDITION 0
    // je  .L1 # False_Label
    // mov eax, TRUE_STMT
    // jmp .L2 # End_Label
    // .L1
    // mov eax, FALSE_STMT
    // .L2
    // mov <dest>, eax
    
    Temp res = tr->getNewTempI4();
    Label False_Label = tr->getNewLabel();
    Label End_Label = tr->getNewLabel();
    e->condition->accept(this);
    // if(condition)
    tr->genJumpOnZero(False_Label, e->condition->ATTR(val));
    // true block
    e->true_brch->accept(this);
    tr->genAssign(res, e->true_brch->ATTR(val));
    tr->genJump(End_Label);
    // else
    // false block
    tr->genMarkLabel(False_Label);
    e->false_brch->accept(this);
    tr->genAssign(res, e->false_brch->ATTR(val));
    tr->genMarkLabel(End_Label);
    // ? : operator should have a return value,
    // so let's assign it.
    e->ATTR(val) = res;
}

/* Translates an entire AST into a Piece list.
 *
 * PARAMETERS:
 *   tree  - the AST
 * RETURNS:
 *   the result Piece list (represented by the first node)
 */
Piece *MindCompiler::translate(ast::Program *tree) {
    TransHelper *helper = new TransHelper(md);

    tree->accept(new Translation(helper));

    return helper->getPiece();
}
