#include "Def.hpp"
#include "syntax.hpp"
#include "expr.hpp"
#include "value.hpp"
#include "RE.hpp"
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;

void REPL(){
    // read - evaluation - print loop
    Assoc global_env = empty();
    bool flag=true;
    std::vector<std::pair<std::string, Expr>> deferred_defines; 
    while (1){ 
        #ifndef ONLINE_JUDGE
        if(flag)std::cout<<"scm> ";
        #endif
        Syntax stx = readSyntax(std :: cin);
        try{
            Expr expr = stx -> parse(global_env);
            Define* define_expr = dynamic_cast<Define*>(expr.get());
            if (define_expr != nullptr) {
                // 如果是define，先收集起来，不立即求值
                deferred_defines.push_back({define_expr->var, define_expr->e});
                flag = false;
                continue;
            } else if (!deferred_defines.empty()) {
                // 遇到非define表达式，先处理所有收集的define
                for (const auto& def : deferred_defines) {
                    // 先创建占位符
                    global_env = extend(def.first, NullV(), global_env);
                }  
                // 然后求值所有define的表达式
                for (const auto& def : deferred_defines) {
                    Value value = def.second->eval(global_env);
                    modify(def.first, value, global_env);
                }
                deferred_defines.clear();
                // 现在处理当前的非define表达式
                Value val = expr -> eval(global_env);
                if (val -> v_type == V_TERMINATE) {
                    break;
                }
                if(expr->e_type==E_VOID||val->v_type!=V_VOID||
                   expr->e_type==E_BEGIN||expr->e_type==E_IF||
                   expr->e_type==E_COND||expr->e_type==E_APPLY) {
                    val -> show(std :: cout);
                    flag=true;
                } else {
                    flag=false;
                }
            } else {
                // 没有收集的define，正常处理
                Value val = expr -> eval(global_env);
                if (val -> v_type == V_TERMINATE) {
                    break;
                }
                
                if(expr->e_type==E_VOID||val->v_type!=V_VOID||
                   expr->e_type==E_BEGIN||expr->e_type==E_IF||
                   expr->e_type==E_COND||expr->e_type==E_APPLY) {
                    val -> show(std :: cout);
                    flag=true;
                } else {
                    flag=false;
                }
            }
        }
        catch (const RuntimeError &RE){
            std :: cout << "RuntimeError"<<std::endl;
            deferred_defines.clear(); // 出错时清空收集的define
            continue;
        }
        if(flag)std::cout<<std::endl;
    }
    
    // 处理文件末尾可能剩余的define
    if (!deferred_defines.empty()) {
        try {
            for (const auto& def : deferred_defines) {
                global_env = extend(def.first, NullV(), global_env);
            }
            
            for (const auto& def : deferred_defines) {
                Value value = def.second->eval(global_env);
                modify(def.first, value, global_env);
            }
        }
        catch (const RuntimeError &RE) {
        }
    }
}

int main(int argc, char *argv[]) {
    REPL();
    std::cout<<std::endl;
    return 0;
}