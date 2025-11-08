/**
 * @file parser.cpp
 * @brief Parsing implementation for Scheme syntax tree to expression tree conversion
 * 
 * This file implements the parsing logic that converts syntax trees into
 * expression trees that can be evaluated.
 * primitive operations, and function applications.
 */

#include "RE.hpp"
#include "Def.hpp"
#include "syntax.hpp"
#include "value.hpp"
#include "expr.hpp"
#include <map>
#include <string>
#include <iostream>

using std::string;
using std::vector;
using std::pair;

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;
extern Syntax readSyntax(std::istream &);
/**
 * @brief Default parse method (should be overridden by subclasses)
 */
Expr Syntax::parse(Assoc &env) {
    throw RuntimeError("Unimplemented parse method");
}

Expr Number::parse(Assoc &env) {
    return Expr(new Fixnum(n));
}

Expr RationalSyntax::parse(Assoc &env) {
    return Expr(new RationalNum(numerator,denominator));
}

Expr SymbolSyntax::parse(Assoc &env) {
    return Expr(new Var(s));
}

Expr StringSyntax::parse(Assoc &env) {
    return Expr(new StringExpr(s));
}

Expr TrueSyntax::parse(Assoc &env) {
    return Expr(new True());
}

Expr FalseSyntax::parse(Assoc &env) {
    return Expr(new False());
}

Expr List::parse(Assoc &env) {
    if (stxs.empty()) {
        return Expr(new ListFunc(vector<Expr>()));
    }
    SymbolSyntax *id = dynamic_cast<SymbolSyntax*>(stxs[0].get());
    if (id != nullptr) {
        std::cout << "DEBUG: List::parse - first element is symbol: " << id->s << std::endl;
    }
    if (id == nullptr) {
        vector<Expr> parameters;
        for(int i=1;i<stxs.size();++i){
            parameters.push_back(stxs[i]->parse(env));
        }
        Expr rat=stxs[0]->parse(env);
        return Expr(new Apply(rat,parameters));
    }else{
    string op = id->s;
    if (find(op, env).get() != nullptr) {
        Value found = find(op, env);
        if (found.get() != nullptr) {
        vector<Expr> parameters;
        for (int i = 1; i < stxs.size(); ++i) {
            parameters.push_back(stxs[i]->parse(env));
        }
            Expr rat = stxs[0]->parse(env);
            return Expr(new Apply(rat, parameters));
        }
    }
    if (primitives.count(op) != 0) {
        vector<Expr> parameters;
        for(int i=1;i<stxs.size();++i){
            parameters.push_back(stxs[i]->parse(env));
        }
        ExprType op_type = primitives[op];
        if (op_type == E_PLUS) {
            if(parameters.empty()){return Expr(new PlusVar({}));}
            else if(parameters.size()==1){return Expr(new Plus(parameters[0],new Fixnum(0)));}
            else if (parameters.size() == 2) {
                return Expr(new Plus(parameters[0], parameters[1])); 
            } else if(parameters.size()>2){
                return Expr(new PlusVar(parameters));
            }
        } else if (op_type == E_MINUS) {
            if(parameters.empty()){throw RuntimeError("Minus: num needed");}
            else if(parameters.size()==1){return Expr(new Minus(new Fixnum(0),parameters[0]));}
            else if (parameters.size() == 2) {
                return Expr(new Minus(parameters[0], parameters[1])); 
            } else if(parameters.size()>2){
                return Expr(new MinusVar(parameters));
            }
        } else if (op_type == E_MUL) {
           if(parameters.empty()){return Expr(new MultVar({}));}
            else if(parameters.size()==1){return Expr(new Mult(new Fixnum(1),parameters[0]));}
            else if (parameters.size() == 2) {
                return Expr(new Mult(parameters[0], parameters[1])); 
            } else if(parameters.size()>2){
                return Expr(new MultVar(parameters));
            }
        }  else if (op_type == E_DIV) {
            if(parameters.empty()){throw RuntimeError("Div: num needed");}
            else if(parameters.size()==1){return Expr(new Div(new Fixnum(1),parameters[0]));}
            else if (parameters.size() == 2) {
                return Expr(new Div(parameters[0], parameters[1])); 
            } else if(parameters.size()>2){
                return Expr(new DivVar(parameters));
            }
        } else if (op_type == E_MODULO) {
            if (parameters.size() != 2) {
                throw RuntimeError("Wrong number of arguments for modulo");
            }
            return Expr(new Modulo(parameters[0], parameters[1]));
        } else if (op_type == E_LIST) {
            return Expr(new ListFunc(parameters));
        } else if (op_type == E_LT) {
            if(parameters.size()<2){throw RuntimeError("More vars needed");}
            else if(parameters.size()==2){return Expr(new Less(parameters[0],parameters[1]));}
            else {return Expr(new LessVar(parameters));}
        } else if (op_type == E_LE) {
            if(parameters.size()<2){throw RuntimeError("More vars needed");}
            else if(parameters.size()==2){return Expr(new LessEq(parameters[0],parameters[1]));}
            return Expr(new LessEqVar(parameters));
        } else if (op_type == E_EQ) {
            if(parameters.size()<2){throw RuntimeError("More vars needed");}
            else if(parameters.size()==2){return Expr(new Equal(parameters[0],parameters[1]));}
            return Expr(new EqualVar(parameters));
        } else if (op_type == E_GE) {
            if(parameters.size()<2){throw RuntimeError("More vars needed");}
            else if(parameters.size()==2){return Expr(new GreaterEq(parameters[0],parameters[1]));}
            return Expr(new GreaterEqVar(parameters));
        } else if (op_type == E_GT) {
            if(parameters.size()<2){throw RuntimeError("More vars needed");}
            else if(parameters.size()==2){return Expr(new Greater(parameters[0],parameters[1]));}
            return Expr(new GreaterVar(parameters));
        } else if (op_type == E_AND) {
            return Expr(new AndVar(parameters));
        } else if (op_type == E_OR) {
            return Expr(new OrVar(parameters));
        } else if (op_type == E_NOT) {
            if(parameters.size()==1){
                return Expr(new Not(parameters[0]));
            }
            else {throw "Wrong number for not";}
        }else if (op_type == E_CONS) {
            if(parameters.size()==2){
                return Expr(new Cons(parameters[0],parameters[1]));
            }
            else {throw "Wrong number for Cons";}
        }else if (op_type == E_CAR) {
            if(parameters.size()==1){
                return Expr(new Car(parameters[0]));
            }
            else {throw "Wrong number for Car";}
        }else if (op_type == E_CDR) {
            if(parameters.size()==1){
                return Expr(new Cdr(parameters[0]));
            }
            else {throw "Wrong number for Cdr";}
        }else if (op_type == E_LIST) {
            return Expr(new ListFunc(parameters));
        }else if (op_type == E_SETCAR) {
            if(parameters.size()==2){
                return Expr(new SetCar(parameters[0],parameters[1]));
            }
            else {throw "Wrong number for SetCar";}
        }else if (op_type == E_SETCDR) {
            if(parameters.size()==2){
                return Expr(new SetCdr(parameters[0],parameters[1]));
            }
            else {throw "Wrong number for SetCdr";}
        }else if (op_type == E_EQQ) {
            if(parameters.size()==2){
                return Expr(new IsEq(parameters[0],parameters[1]));
            }
            else {throw "Wrong number for IsEq";}
        }else if (op_type == E_BOOLQ) {
            if(parameters.size()==1){
                return Expr(new IsBoolean(parameters[0]));
            }
            else {throw "Wrong number for IsBoolean";}
        }else if (op_type == E_INTQ) {
            if(parameters.size()==1){
                return Expr(new IsFixnum(parameters[0]));
            }
            else {throw "Wrong number for IsInt";}
        }else if (op_type == E_NULLQ) {
            if(parameters.size()==1){
                return Expr(new IsNull(parameters[0]));
            }
            else {throw "Wrong number for IsNull";}
        }else if (op_type == E_PAIRQ) {
            if(parameters.size()==1){
                return Expr(new IsPair(parameters[0]));
            }
            else {throw "Wrong number for IsPair";}
        }else if (op_type == E_PROCQ) {
            if(parameters.size()==1){
                return Expr(new IsProcedure(parameters[0]));
            }
            else {throw "Wrong number for IsProcedure";}
        }else if (op_type == E_SYMBOLQ) {
            if(parameters.size()==1){
                return Expr(new IsSymbol(parameters[0]));
            }
            else {throw "Wrong number for IsSymbol";}
        }else if (op_type == E_LISTQ) {
            if(parameters.size()==1){
                return Expr(new IsList(parameters[0]));
            }
            else {throw "Wrong number for IsList";}
        }else if (op_type == E_STRINGQ) {
            if(parameters.size()==1){
                return Expr(new IsString(parameters[0]));
            }
            else {throw "Wrong number for IsString";}
        }else if (op_type == E_DISPLAY) {
           if(parameters.size()!=1){throw RuntimeError("Display: 1 param needed");}
           return Expr(new Display(parameters[0]));
        }else if (op_type == E_VOID) {
           return Expr(new MakeVoid());
        }else if (op_type == E_EXIT) {
            return Expr(new Exit());
        }else {
            throw RuntimeError ("Unknown primitive:" + op);
        }
    }

    if (reserved_words.count(op) != 0) {
        if(op=="begin"){
            vector<Expr> parameters;
            for(int i=1;i<stxs.size();++i){
                parameters.push_back(stxs[i]->parse(env));
            }
            return Expr(new Begin(parameters));
        }
        if(op=="quote"){
            if (stxs.size() != 2) {
                throw RuntimeError("quote: requires 1 argument");
            }
            return Expr(new Quote(stxs[1]));
        }
        if(op=="if"){
            if (stxs.size() <3||stxs.size()>4) {
                throw RuntimeError("if: requires 2 or 3 argument");
            }
            vector<Expr> parameters;
            for(int i=1;i<stxs.size();++i){
                parameters.push_back(stxs[i]->parse(env));
            }
            Expr alter = stxs.size()==4 ? parameters[2] : Expr(new MakeVoid());
            return Expr(new If(parameters[0],parameters[1],alter));
        }
        if(op=="cond"){
            vector<vector<Expr>> tmp;
            for (int i=1;i<stxs.size();++i){
                auto clause_list=dynamic_cast<List*>(stxs[i].get());
                if(clause_list==nullptr){throw RuntimeError("cond:list needed");}
                vector<Expr> clauses;
                for(auto& item : clause_list->stxs){
                    clauses.push_back(item->parse(env));
                }
                tmp.push_back(clauses);
            }
            return Expr(new Cond(tmp));
        }
        if(op=="lambda"){
            if (stxs.size() < 3) {
                throw RuntimeError("lambda: requires parameter list and body");
            }
            auto paramList = dynamic_cast<List*>(stxs[1].get());
            if (paramList==nullptr) {
                throw RuntimeError("lambda: parameter list must be a list");
            }
            vector<string> params;
            for (auto& param : paramList->stxs) {
                auto sym = dynamic_cast<SymbolSyntax*>(param.get());
                if (sym==nullptr) {
                    throw RuntimeError("lambda: parameters must be symbols");
                }
                params.push_back(sym->s);
            }
            vector<Expr> bodyExprs;
            for (int i = 2; i < stxs.size(); ++i) {
                bodyExprs.push_back(stxs[i]->parse(env));
            }
            Expr body(nullptr);
            if (bodyExprs.size() == 1) {
                body = bodyExprs[0];
            } else {
                body = Expr(new Begin(bodyExprs));
            }
            return Expr(new Lambda(params, body));
        }
        if(op=="define"){
            if(stxs.size()<3){
                throw RuntimeError("define: requires at least 2 augments");
            }
            auto varSym = dynamic_cast<SymbolSyntax*>(stxs[1].get());
            if(varSym!=nullptr){
                vector<Expr> parameters;
                for(int i=1;i<stxs.size();++i){
                    parameters.push_back(stxs[i]->parse(env));
                }
                if(parameters.size()!=2){throw RuntimeError("define: only 1 var can be defined");}
                return Expr(new Define(varSym->s,parameters[1]));
            }//定义的是一个变量
            else{
                auto funcDef = dynamic_cast<List*>(stxs[1].get());
                if (funcDef==nullptr || funcDef->stxs.empty()) {
                    throw RuntimeError("define: malformed function definition");
                }
                auto funcNameSym = dynamic_cast<SymbolSyntax*>(funcDef->stxs[0].get());
                if (funcNameSym==nullptr) {
                    throw RuntimeError("define: function name must be a symbol");
                }
                vector<string> params;
                for (int i = 1; i < funcDef->stxs.size(); ++i) {
                    auto paramSym = dynamic_cast<SymbolSyntax*>(funcDef->stxs[i].get());
                    if (paramSym==nullptr) {
                        throw RuntimeError("define: parameters must be symbols");
                    }
                    params.push_back(paramSym->s);
                }
                vector<Expr> bodyExprs;
                for(int i=2;i<stxs.size();++i){
                    bodyExprs.push_back(stxs[i]->parse(env));
                }
                Expr body(nullptr);
                if(bodyExprs.size()==1){
                    body=bodyExprs[0];
                }else{
                    body=Expr(new Begin(bodyExprs));
                }
                Expr lambda = Expr(new Lambda(params, body));
                return Expr(new Define(funcNameSym->s, lambda));
            }//定义的是一个函数
        }
        if(op=="set!"){
            if (stxs.size() != 3) {
                throw RuntimeError("set!: requires 2 arguments");
            }
            auto varSym = dynamic_cast<SymbolSyntax*>(stxs[1].get());
            if (varSym==nullptr) {
                throw RuntimeError("set!: first argument must be a variable");
            }
            Expr value =stxs[2]->parse(env);
            return Expr(new Set(varSym->s, value));
        }     
        if (op == "let") {
            if (stxs.size() < 2) {
                throw RuntimeError("let: requires bindings and body");
            }
            auto bindings_list = dynamic_cast<List*>(stxs[1].get());
            if (bindings_list==nullptr) {
                throw RuntimeError("let: bindings must be a list");
            }
            std::vector<std::pair<std::string, Expr>> bindings;
            for (auto& binding_syntax : bindings_list->stxs) {
                auto binding = dynamic_cast<List*>(binding_syntax.get());
                if (binding==nullptr || binding->stxs.size() != 2) {
                    throw RuntimeError("let: each binding must be (variable value)");
                }
                auto var_sym = dynamic_cast<SymbolSyntax*>(binding->stxs[0].get());
                if (var_sym==nullptr) {
                    throw RuntimeError("let: variable must be a symbol");
                }
                Expr value_expr = binding->stxs[1]->parse(env);
                bindings.push_back({var_sym->s, value_expr});
            }
            std::vector<Expr> body_exprs;
            for (size_t i = 2; i < stxs.size(); i++) {
                body_exprs.push_back(stxs[i]->parse(env));
            }
            Expr body(nullptr);
            if (body_exprs.size() == 1) {
                body = body_exprs[0];
            } else {
                body = Expr(new Begin(body_exprs));
            }
            return Expr(new Let(bindings, body));
        }
        if (op == "letrec") {
            if (stxs.size() < 2) {
                throw RuntimeError("letrec: requires bindings and body");
            }
            auto bindings_list = dynamic_cast<List*>(stxs[1].get());
            if (bindings_list==nullptr) {
                throw RuntimeError("letrec: bindings must be a list");
            }
    
            std::vector<std::pair<std::string, Expr>> bindings;
            for (auto& binding_syntax : bindings_list->stxs) {
                auto binding = dynamic_cast<List*>(binding_syntax.get());
                if (binding==nullptr || binding->stxs.size() != 2) {
                    throw RuntimeError("letrec: each binding must be (variable value)");
                }
        
                auto var_sym = dynamic_cast<SymbolSyntax*>(binding->stxs[0].get());
                if (var_sym==nullptr) {
                    throw RuntimeError("letrec: variable must be a symbol");
                }
        
                Expr value_expr = binding->stxs[1]->parse(env);
                bindings.push_back({var_sym->s, value_expr});
            }
            std::vector<Expr> body_exprs;
            for (size_t i = 2; i < stxs.size(); i++) {
                body_exprs.push_back(stxs[i]->parse(env));
            }
            Expr body(nullptr);
            if (body_exprs.size() == 1) {
                body = body_exprs[0];
            } else {
                body = Expr(new Begin(body_exprs));
            }
            return Expr(new Letrec(bindings, body));
        }
    }
        vector<Expr> parameters;
        for(int i=1;i<stxs.size();++i){
            parameters.push_back(stxs[i]->parse(env));
        }
        Expr rat=stxs[0]->parse(env);
        return Expr(new Apply(rat,parameters));
    }
}
