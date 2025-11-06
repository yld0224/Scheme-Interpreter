#include "Def.hpp"
#include "syntax.hpp"
#include "expr.hpp"
#include "value.hpp"
#include "RE.hpp"
#include <sstream>
#include <iostream>
#include <map>

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;

bool isExplicitVoidCall(Expr expr) {
    MakeVoid* make_void_expr = dynamic_cast<MakeVoid*>(expr.get());
    if (make_void_expr != nullptr) {
        return true;
    }
    
    Apply* apply_expr = dynamic_cast<Apply*>(expr.get());
    if (apply_expr != nullptr) {
        Var* var_expr = dynamic_cast<Var*>(apply_expr->rator.get());
        if (var_expr != nullptr && var_expr->x == "void") {
            return true;
        }
    }
    
    Begin* begin_expr = dynamic_cast<Begin*>(expr.get());
    if (begin_expr != nullptr && !begin_expr->es.empty()) {
        return isExplicitVoidCall(begin_expr->es.back());
    }
    
    If* if_expr = dynamic_cast<If*>(expr.get());
    if (if_expr != nullptr) {
        return isExplicitVoidCall(if_expr->conseq) || isExplicitVoidCall(if_expr->alter);
    }
    
    Cond* cond_expr = dynamic_cast<Cond*>(expr.get());
    if (cond_expr != nullptr) {
        for (const auto& clause : cond_expr->clauses) {
            if (clause.size() > 1 && isExplicitVoidCall(clause.back())) {
                return true;
            }
        }
    }
    return false;
}

void REPL(){
    Assoc global_env = empty();
    while (1){
        #ifndef ONLINE_JUDGE
            std::cout << "scm> ";
        #endif
        Syntax stx = readSyntax(std :: cin);
        try{
            //std::cout << "DEBUG: Syntax parsed" << std::endl;
            Expr expr = stx -> parse(global_env);
            //std::cout << "DEBUG: Expression parsed" << std::endl;
            Value val = expr -> eval(global_env);
            //std::cout << "DEBUG: Evaluation completed" << std::endl;
            
            if (val -> v_type == V_TERMINATE)
                break;
                
            //std::cout << "DEBUG: Showing value: ";
            val -> show(std :: cout);
            //std::cout << std::endl;
        }
        catch (const RuntimeError &RE){
           // std::cout << "RuntimeError: " << RE.message() << std::endl;
        }
        catch (const std::exception &e) {
           // std::cout << "Std exception: " << e.what() << std::endl;
        }
        catch (...) {
            //std::cout << "Unknown exception" << std::endl;
        }
        puts("");
    }
}


int main(int argc, char *argv[]) {
    REPL();
    return 0;
}
