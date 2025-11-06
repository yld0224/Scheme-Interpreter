/**
 * @file evaluation.cpp
 * @brief Expression evaluation implementation for the Scheme interpreter
 * @author luke36
 * 
 * This file implements evaluation methods for all expression types in the Scheme
 * interpreter. Functions are organized according to ExprType enumeration order
 * from Def.hpp for consistency and maintainability.
 */
#include <sstream>
#include "value.hpp"
#include "expr.hpp" 
#include "RE.hpp"
#include "syntax.hpp"
#include <cstring>
#include <vector>
#include <map>
#include <climits>

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;

Value Fixnum::eval(Assoc &e) { // evaluation of a fixnum
    return IntegerV(n);
}

Value RationalNum::eval(Assoc &e) { // evaluation of a rational number
    return RationalV(numerator, denominator);
}

Value StringExpr::eval(Assoc &e) { // evaluation of a string
    return StringV(s);
}

Value True::eval(Assoc &e) { // evaluation of #t
    return BooleanV(true);
}

Value False::eval(Assoc &e) { // evaluation of #f
    return BooleanV(false);
}

Value MakeVoid::eval(Assoc &e) { // (void)
    return VoidV();
}

Value Exit::eval(Assoc &e) { // (exit)
    return TerminateV();
}

Value Unary::eval(Assoc &e) { // evaluation of single-operator primitive
    return evalRator(rand->eval(e));
}

Value Binary::eval(Assoc &e) { // evaluation of two-operators primitive
    return evalRator(rand1->eval(e), rand2->eval(e));
}

Value Variadic::eval(Assoc &e) {
    std::vector<Value> tmp;
    for(auto i:rands){
        tmp.push_back(i->eval(e));
    }
    return evalRator(tmp);
}//实现了一些eval求值

double isNumber(const std::string& str){
    std::istringstream iss(str);
    double num;
    if(iss>>num){
        return num;
    }
    return -1;
}//用来判断x是不是一个数字
Value Var::eval(Assoc &e) { 
    if(isNumber(x)!=-1){return IntegerV(isNumber(x));}
    if(x[0]=='.'||x[0]=='@'||x[0]=='0'||x[0]=='1'
        ||x[0]=='2'||x[0]=='3'||x[0]=='4'||
        x[0]=='5'||x[0]=='6'||x[0]=='7'|x[0]=='8'||x[0]=='9'){throw RuntimeError("Invalid variable name");}
    for(auto i:x){
        if(i=='#'||i=='\''||i=='"'||i=='`'){
            throw RuntimeError("Invalid variable name");
        }
    }
    
    Value matched_value = find(x, e);
    if (matched_value.get() == nullptr) {
        if (primitives.count(x)) {
             static std::map<ExprType, std::pair<Expr, std::vector<std::string>>> primitive_map = {
                    {E_VOID,     {new MakeVoid(), {}}},
                    {E_EXIT,     {new Exit(), {}}},
                    {E_BOOLQ,    {new IsBoolean(new Var("parm")), {"parm"}}},
                    {E_INTQ,     {new IsFixnum(new Var("parm")), {"parm"}}},
                    {E_NULLQ,    {new IsNull(new Var("parm")), {"parm"}}},
                    {E_PAIRQ,    {new IsPair(new Var("parm")), {"parm"}}},
                    {E_PROCQ,    {new IsProcedure(new Var("parm")), {"parm"}}},
                    {E_SYMBOLQ,  {new IsSymbol(new Var("parm")), {"parm"}}},
                    {E_STRINGQ,  {new IsString(new Var("parm")), {"parm"}}},
                    {E_DISPLAY,  {new Display(new Var("parm")), {"parm"}}},
                    {E_PLUS,     {new PlusVar({}),  {}}},
                    {E_MINUS,    {new MinusVar({}), {}}},
                    {E_MUL,      {new MultVar({}),  {}}},
                    {E_DIV,      {new DivVar({}),   {}}},
                    {E_MODULO,   {new Modulo(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_EXPT,     {new Expt(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_EQQ,      {new EqualVar({}), {}}},
            };

            auto it = primitive_map.find(primitives[x]);
            if (it != primitive_map.end()) {
                if(it->first==E_VOID){
                    auto ptr=dynamic_cast<MakeVoid*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_EXIT){
                    auto ptr=dynamic_cast<Exit*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_BOOLQ){
                    auto ptr=dynamic_cast<IsBoolean*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_INTQ){
                    auto ptr=dynamic_cast<IsFixnum*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_NULLQ){
                    auto ptr=dynamic_cast<IsNull*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_PAIRQ){
                    auto ptr=dynamic_cast<IsPair*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_SYMBOLQ){
                    auto ptr=dynamic_cast<IsSymbol*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_STRINGQ){
                    auto ptr=dynamic_cast<IsString*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_PROCQ){
                    auto ptr=dynamic_cast<IsProcedure*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_DISPLAY){
                    auto ptr=dynamic_cast<Display*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_PLUS){
                    auto ptr=dynamic_cast<PlusVar*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_DIV){
                    auto ptr=dynamic_cast<DivVar*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_MODULO){
                    auto ptr=dynamic_cast<Modulo*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_MINUS){
                    auto ptr=dynamic_cast<MinusVar*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_MUL){
                    auto ptr=dynamic_cast<MultVar*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_EXPT){
                    auto ptr=dynamic_cast<Expt*>(it->second.first.get());
                    return ptr->eval(e);
                }if(it->first==E_EQQ){
                    auto ptr=dynamic_cast<EqualVar*>(it->second.first.get());
                    return ptr->eval(e);
                }
            }
        }
        throw RuntimeError("Undefined Variable");
    }
    return matched_value;
}

static int gcd(int a, int b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

Value Plus::evalRator(const Value &rand1, const Value &rand2) {
    ValueType type1=rand1->v_type;
    ValueType type2=rand2->v_type;
    if((type1!=V_RATIONAL&&type1!=V_INT)||(type2!=V_RATIONAL&&type2!=V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    else if(type1==V_RATIONAL&&type2==V_RATIONAL){
        auto ptr1=dynamic_cast<Rational*>(rand1.get());
        auto ptr2=dynamic_cast<Rational*>(rand2.get());
        int a=ptr1->numerator*ptr2->denominator+ptr1->denominator*ptr2->numerator;
        int b=ptr1->denominator*ptr2->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
    else if(type1==V_INT&&type2==V_INT){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int num=(ptr1->n)+(ptr2->n);
        return IntegerV(num);
    }
    else if(type1==V_INT&&type2==V_RATIONAL){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Rational*>(rand2.get());
        int a=ptr1->n*ptr2->denominator+ptr2->numerator;
        int b=ptr2->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
     else if(type1==V_RATIONAL&&type2==V_INT){
        auto ptr1=static_cast<Rational*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int a=ptr2->n*ptr1->denominator+ptr1->numerator;
        int b=ptr1->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
}
Value plus(const Value &rand1,const Value &rand2){
    ValueType type1=rand1->v_type;
    ValueType type2=rand2->v_type;
    if((type1!=V_RATIONAL&&type1!=V_INT)||(type2!=V_RATIONAL&&type2!=V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    else if(type1==V_RATIONAL&&type2==V_RATIONAL){
        auto ptr1=dynamic_cast<Rational*>(rand1.get());
        auto ptr2=dynamic_cast<Rational*>(rand2.get());
        int a=ptr1->numerator*ptr2->denominator+ptr1->denominator*ptr2->numerator;
        int b=ptr1->denominator*ptr2->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
    else if(type1==V_INT&&type2==V_INT){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int num=(ptr1->n)+(ptr2->n);
        return IntegerV(num);
    }
    else if(type1==V_INT&&type2==V_RATIONAL){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Rational*>(rand2.get());
        int a=ptr1->n*ptr2->denominator+ptr2->numerator;
        int b=ptr2->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
     else if(type1==V_RATIONAL&&type2==V_INT){
        auto ptr1=static_cast<Rational*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int a=ptr2->n*ptr1->denominator+ptr1->numerator;
        int b=ptr1->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
}//用于多参数的plus

Value Minus::evalRator(const Value &rand1, const Value &rand2) { // -
    ValueType type1=rand1->v_type;
    ValueType type2=rand2->v_type;
    if((type1!=V_RATIONAL&&type1!=V_INT)||(type2!=V_RATIONAL&&type2!=V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    else if(type1==V_RATIONAL&&type2==V_RATIONAL){
        auto ptr1=dynamic_cast<Rational*>(rand1.get());
        auto ptr2=dynamic_cast<Rational*>(rand2.get());
        int a=ptr1->numerator*ptr2->denominator-ptr1->denominator*ptr2->numerator;
        int b=ptr1->denominator*ptr2->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
    else if(type1==V_INT&&type2==V_INT){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int num=(ptr1->n)-(ptr2->n);
        return IntegerV(num);
    }
    else if(type1==V_INT&&type2==V_RATIONAL){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Rational*>(rand2.get());
        int a=ptr1->n*ptr2->denominator-ptr2->numerator;
        int b=ptr2->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
     else if(type1==V_RATIONAL&&type2==V_INT){
        auto ptr1=static_cast<Rational*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int a=ptr2->n*ptr1->denominator-ptr1->numerator;
        int b=ptr1->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
}
Value minus(const Value &rand1, const Value &rand2) { // -
    ValueType type1=rand1->v_type;
    ValueType type2=rand2->v_type;
    if((type1!=V_RATIONAL&&type1!=V_INT)||(type2!=V_RATIONAL&&type2!=V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    else if(type1==V_RATIONAL&&type2==V_RATIONAL){
        auto ptr1=dynamic_cast<Rational*>(rand1.get());
        auto ptr2=dynamic_cast<Rational*>(rand2.get());
        int a=ptr1->numerator*ptr2->denominator-ptr1->denominator*ptr2->numerator;
        int b=ptr1->denominator*ptr2->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
    else if(type1==V_INT&&type2==V_INT){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int num=(ptr1->n)-(ptr2->n);
        return IntegerV(num);
    }
    else if(type1==V_INT&&type2==V_RATIONAL){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Rational*>(rand2.get());
        int a=ptr1->n*ptr2->denominator-ptr2->numerator;
        int b=ptr2->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
     else if(type1==V_RATIONAL&&type2==V_INT){
        auto ptr1=static_cast<Rational*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int a=ptr2->n*ptr1->denominator-ptr1->numerator;
        int b=ptr1->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
}

Value Mult::evalRator(const Value &rand1, const Value &rand2) { // *
   ValueType type1=rand1->v_type;
    ValueType type2=rand2->v_type;
    if((type1!=V_RATIONAL&&type1!=V_INT)||(type2!=V_RATIONAL&&type2!=V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    else if(type1==V_RATIONAL&&type2==V_RATIONAL){
        auto ptr1=dynamic_cast<Rational*>(rand1.get());
        auto ptr2=dynamic_cast<Rational*>(rand2.get());
        int a=ptr1->numerator*ptr2->numerator;
        int b=ptr1->denominator*ptr2->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
    else if(type1==V_INT&&type2==V_INT){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int num=(ptr1->n)*(ptr2->n);
        return IntegerV(num);
    }
    else if(type1==V_INT&&type2==V_RATIONAL){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Rational*>(rand2.get());
        int a=ptr1->n*ptr2->numerator;
        int b=ptr2->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        if(b!=1){return RationalV(a,b);}
        else{return IntegerV(a);}
    }
     else if(type1==V_RATIONAL&&type2==V_INT){
        auto ptr1=static_cast<Rational*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int a=ptr2->n*ptr1->numerator;
        int b=ptr1->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        if(b!=1){return RationalV(a,b);}
        else{return IntegerV(a);}
    }
}
Value mult(const Value &rand1, const Value &rand2) { // *
   ValueType type1=rand1->v_type;
    ValueType type2=rand2->v_type;
    if((type1!=V_RATIONAL&&type1!=V_INT)||(type2!=V_RATIONAL&&type2!=V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    else if(type1==V_RATIONAL&&type2==V_RATIONAL){
        auto ptr1=dynamic_cast<Rational*>(rand1.get());
        auto ptr2=dynamic_cast<Rational*>(rand2.get());
        int a=ptr1->numerator*ptr2->numerator;
        int b=ptr1->denominator*ptr2->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        return RationalV(a,b);
    }
    else if(type1==V_INT&&type2==V_INT){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int num=(ptr1->n)*(ptr2->n);
        return IntegerV(num);
    }
    else if(type1==V_INT&&type2==V_RATIONAL){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Rational*>(rand2.get());
        int a=ptr1->n*ptr2->numerator;
        int b=ptr2->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        if(b!=1){return RationalV(a,b);}
        else{return IntegerV(a);}
    }
     else if(type1==V_RATIONAL&&type2==V_INT){
        auto ptr1=static_cast<Rational*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int a=ptr2->n*ptr1->numerator;
        int b=ptr1->denominator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        if(b!=1){return RationalV(a,b);}
        else{return IntegerV(a);}
    }
}
Value Div::evalRator(const Value &rand1, const Value &rand2) { // /
    ValueType type1=rand1->v_type;
    ValueType type2=rand2->v_type;
    if((type1!=V_RATIONAL&&type1!=V_INT)||(type2!=V_RATIONAL&&type2!=V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    else if(type1==V_RATIONAL&&type2==V_RATIONAL){
        auto ptr1=dynamic_cast<Rational*>(rand1.get());
        auto ptr2=dynamic_cast<Rational*>(rand2.get());
        int a=ptr1->numerator*ptr2->denominator;
        int b=ptr1->denominator*ptr2->numerator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        if(b!=1){return RationalV(a,b);}
        else{return IntegerV(a);}
    }
    else if(type1==V_INT&&type2==V_INT){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int num=gcd(ptr1->n,ptr2->n);
        if(ptr2->n==0){throw(RuntimeError("Division by zero"));}
        if(num==ptr2->n){return IntegerV((ptr1->n)/num);}
        else {return RationalV((ptr1->n)/num,(ptr2->n)/num);}
    }
    else if(type1==V_INT&&type2==V_RATIONAL){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Rational*>(rand2.get());
        int a=ptr1->n*ptr2->denominator;
        int b=ptr2->numerator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        if(b!=1){return RationalV(a,b);}
        else{return IntegerV(a);}
    }
     else if(type1==V_RATIONAL&&type2==V_INT){
        auto ptr1=static_cast<Rational*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int a=ptr2->n*ptr1->denominator;
        int b=ptr1->numerator;
        if(a==0){throw(RuntimeError("Division by zero"));}
        int g=gcd(a,b);
        a=a/g;b=b/g;
        {return RationalV(b,a);}
    }
}
Value div(const Value &rand1, const Value &rand2) { // /
    ValueType type1=rand1->v_type;
    ValueType type2=rand2->v_type;
    if((type1!=V_RATIONAL&&type1!=V_INT)||(type2!=V_RATIONAL&&type2!=V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    else if(type1==V_RATIONAL&&type2==V_RATIONAL){
        auto ptr1=dynamic_cast<Rational*>(rand1.get());
        auto ptr2=dynamic_cast<Rational*>(rand2.get());
        int a=ptr1->numerator*ptr2->denominator;
        int b=ptr1->denominator*ptr2->numerator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        if(b!=1){return RationalV(a,b);}
        else{return IntegerV(a);}
    }
    else if(type1==V_INT&&type2==V_INT){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int num=gcd(ptr1->n,ptr2->n);
        if(ptr2->n==0){throw(RuntimeError("Division by zero"));}
        if(ptr1->n%ptr2->n==0){return IntegerV((ptr1->n)/(ptr2->n));}
        else {return RationalV((ptr1->n)/num,(ptr2->n)/num);}
    }
    else if(type1==V_INT&&type2==V_RATIONAL){
        auto ptr1=static_cast<Integer*>(rand1.get());
        auto ptr2=static_cast<Rational*>(rand2.get());
        int a=ptr1->n*ptr2->denominator;
        int b=ptr2->numerator;
        int g=gcd(a,b);
        a=a/g;b=b/g;
        if(b!=1){return RationalV(a,b);}
        else{return IntegerV(a);}
    }
     else if(type1==V_RATIONAL&&type2==V_INT){
        auto ptr1=static_cast<Rational*>(rand1.get());
        auto ptr2=static_cast<Integer*>(rand2.get());
        int a=ptr2->n*ptr1->denominator;
        int b=ptr1->numerator;
        if(a==0){throw(RuntimeError("Division by zero"));}
        int g=gcd(a,b);
        a=a/g;b=b/g;
        {return RationalV(b,a);}
    }
}
Value Modulo::evalRator(const Value &rand1, const Value &rand2) { // modulo
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        int dividend = dynamic_cast<Integer*>(rand1.get())->n;
        int divisor = dynamic_cast<Integer*>(rand2.get())->n;
        if (divisor == 0) {
            throw(RuntimeError("Division by zero"));
        }
        return IntegerV(dividend % divisor);
    }
    throw(RuntimeError("modulo is only defined for integers"));
}

Value PlusVar::evalRator(const std::vector<Value> &args) {
    if(args.size()==0){return IntegerV(0);}
    else if(args.size()==1){return args[0];}
    else if(args.size()==2){return plus(args[0],args[1]);}
    else {
        Value sum=plus(args[0],args[1]);
        for(int i=2;i<args.size();++i){
            sum=plus(sum,args[i]);
        }
        return sum;
    }
}

Value MinusVar::evalRator(const std::vector<Value> &args) {
    if(args.size()==0){throw "Runtime Error";} // - with multiple args
    else if(args.size()==1){
        if(args[0]->v_type==V_INT){
            auto ptr=dynamic_cast<Integer*>(args[0].get());
            return IntegerV(-(ptr->n));
        }
        else{
            auto ptr=dynamic_cast<Rational*>(args[0].get());
            return RationalV(-(ptr->numerator),ptr->denominator);
        }
    }
    else if(args.size()==2){return minus(args[0],args[1]);}
    else {
        Value sum=minus(args[0],args[1]);
        for(int i=2;i<args.size();++i){
            sum=minus(sum,args[i]);
        }
        return sum;
    }
}

Value MultVar::evalRator(const std::vector<Value> &args) {
    if(args.size()==0){return IntegerV(1);} // * with multiple args
    else if(args.size()==1){return args[0];}
    else if(args.size()==2){return mult(args[0],args[1]);}
    else {
        Value sum=mult(args[0],args[1]);
        for(int i=2;i<args.size();++i){
            sum=mult(sum,args[i]);
        }
        return sum;
    }
}

Value DivVar::evalRator(const std::vector<Value> &args) {
    if(args.size()==0){throw "Runtime Error";} // / with multiple args
    else if(args.size()==1){return div(IntegerV(1),args[0]);}
    else if(args.size()==2){return div(args[0],args[1]);}
    else {
        Value sum=div(args[0],args[1]);
        for(int i=2;i<args.size();++i){
            sum=div(sum,args[i]);
        }
        return sum;
    }
}

Value Expt::evalRator(const Value &rand1, const Value &rand2) { // expt
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        int base = dynamic_cast<Integer*>(rand1.get())->n;
        int exponent = dynamic_cast<Integer*>(rand2.get())->n;
        
        if (exponent < 0) {
            throw(RuntimeError("Negative exponent not supported for integers"));
        }
        if (base == 0 && exponent == 0) {
            throw(RuntimeError("0^0 is undefined"));
        }
        
        long long result = 1;
        long long b = base;
        int exp = exponent;
        
        while (exp > 0) {
            if (exp % 2 == 1) {
                result *= b;
                if (result > INT_MAX || result < INT_MIN) {
                    throw(RuntimeError("Integer overflow in expt"));
                }
            }
            b *= b;
            if (b > INT_MAX || b < INT_MIN) {
                if (exp > 1) {
                    throw(RuntimeError("Integer overflow in expt"));
                }
            }
            exp /= 2;
        }
        
        return IntegerV((int)result);
    }
    throw(RuntimeError("Wrong typename"));
}

//A FUNCTION TO SIMPLIFY THE COMPARISON WITH INTEGER AND RATIONAL NUMBER
int compareNumericValues(const Value &v1, const Value &v2) {
    if (v1->v_type == V_INT && v2->v_type == V_INT) {
        int n1 = dynamic_cast<Integer*>(v1.get())->n;
        int n2 = dynamic_cast<Integer*>(v2.get())->n;
        return (n1 < n2) ? -1 : (n1 > n2) ? 1 : 0;
    }
    else if (v1->v_type == V_RATIONAL && v2->v_type == V_INT) {
        Rational* r1 = dynamic_cast<Rational*>(v1.get());
        int n2 = dynamic_cast<Integer*>(v2.get())->n;
        int left = r1->numerator;
        int right = n2 * r1->denominator;
        return (left < right) ? -1 : (left > right) ? 1 : 0;
    }
    else if (v1->v_type == V_INT && v2->v_type == V_RATIONAL) {
        int n1 = dynamic_cast<Integer*>(v1.get())->n;
        Rational* r2 = dynamic_cast<Rational*>(v2.get());
        int left = n1 * r2->denominator;
        int right = r2->numerator;
        return (left < right) ? -1 : (left > right) ? 1 : 0;
    }
    else if (v1->v_type == V_RATIONAL && v2->v_type == V_RATIONAL) {
        Rational* r1 = dynamic_cast<Rational*>(v1.get());
        Rational* r2 = dynamic_cast<Rational*>(v2.get());
        int left = r1->numerator * r2->denominator;
        int right = r2->numerator * r1->denominator;
        return (left < right) ? -1 : (left > right) ? 1 : 0;
    }
    throw RuntimeError("Wrong typename in numeric comparison");
}

Value Less::evalRator(const Value &rand1, const Value &rand2) { // <
    if((rand1->v_type!=V_RATIONAL&&rand1->v_type!=V_INT)
    ||(rand2->v_type!=V_RATIONAL&&rand2->v_type!=V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    if(compareNumericValues(rand1,rand2)==-1){return BooleanV(true);}
    else{return BooleanV(false);}
}

Value LessEq::evalRator(const Value &rand1, const Value &rand2) { // <=
   if((rand1->v_type!=V_RATIONAL&&rand1->v_type!=V_INT)
    ||(rand2->v_type!=V_RATIONAL&&rand2->v_type!=V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    if(compareNumericValues(rand1,rand2)==-1
    ||compareNumericValues(rand1,rand2)==0){return BooleanV(true);}
    else{return BooleanV(false);}
}

Value Equal::evalRator(const Value &rand1, const Value &rand2) { // =
    if((rand1->v_type!=V_RATIONAL&&rand1->v_type!=V_INT)
    ||(rand2->v_type!=V_RATIONAL&&rand2->v_type!=V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    if(compareNumericValues(rand1,rand2)==0){return BooleanV(true);}
    else{return BooleanV(false);}
}

Value GreaterEq::evalRator(const Value &rand1, const Value &rand2) { // >=
   if((rand1->v_type!=V_RATIONAL&&rand1->v_type!=V_INT)
    ||(rand2->v_type!=V_RATIONAL&&rand2->v_type!=V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    if(compareNumericValues(rand1,rand2)==0
    ||compareNumericValues(rand1,rand2)==1){return BooleanV(true);}
    else{return BooleanV(false);}
}

Value Greater::evalRator(const Value &rand1, const Value &rand2) { // >
    if((rand1->v_type!=V_RATIONAL&&rand1->v_type!=V_INT)
    ||(rand2->v_type!=V_RATIONAL&&rand2->v_type!=V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    if(compareNumericValues(rand1,rand2)==1){return BooleanV(true);}
    else{return BooleanV(false);}
}

Value LessVar::evalRator(const std::vector<Value> &args) {
    for(int i=0;i<args.size();++i){
        if(args[i]->v_type!=V_RATIONAL&&args[i]->v_type!=V_INT){
            throw(RuntimeError("Wrong typename"));
        }
    } // < with multiple args
    for(int i=0;i<args.size()-1;++i){
        if(compareNumericValues(args[i],args[i+1])!=-1){
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value LessEqVar::evalRator(const std::vector<Value> &args) {
    for(int i=0;i<args.size();++i){
        if(args[i]->v_type!=V_RATIONAL&&args[i]->v_type!=V_INT){
            throw(RuntimeError("Wrong typename"));
        }
    } // <= with multiple args
    for(int i=0;i<args.size()-1;++i){
        if(compareNumericValues(args[i],args[i+1])==1){
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value EqualVar::evalRator(const std::vector<Value> &args) {
    for(int i=0;i<args.size();++i){
        if(args[i]->v_type!=V_RATIONAL&&args[i]->v_type!=V_INT){
            throw(RuntimeError("Wrong typename"));
        }
    } // = with multiple args
    for(int i=0;i<args.size()-1;++i){
        if(compareNumericValues(args[i],args[i+1])!=0){
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value GreaterEqVar::evalRator(const std::vector<Value> &args) {
    for(int i=0;i<args.size();++i){
        if(args[i]->v_type!=V_RATIONAL&&args[i]->v_type!=V_INT){
            throw(RuntimeError("Wrong typename"));
        }
    } // >= with multiple args
    for(int i=0;i<args.size()-1;++i){
        if(compareNumericValues(args[i],args[i+1])==-1){
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value GreaterVar::evalRator(const std::vector<Value> &args) {
    for(int i=0;i<args.size();++i){
        if(args[i]->v_type!=V_RATIONAL&&args[i]->v_type!=V_INT){
            throw(RuntimeError("Wrong typename"));
        }
    } // > with multiple args
    for(int i=0;i<args.size()-1;++i){
        if(compareNumericValues(args[i],args[i+1])!=1){
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value Cons::evalRator(const Value &rand1, const Value &rand2) { // cons
    return PairV(rand1,rand2);
}

Value ListFunc::evalRator(const std::vector<Value> &args) { // list function
    if(args.size()==0){return NullV();}
    else if(args.size()==1){return PairV(args[0],NullV());}
    else{
        Value v=PairV(args[0],NullV());
        auto ptr=dynamic_cast<Pair*>(v.get());
        for(auto iter=args.begin()+1;iter!=args.end();++iter){
            ptr->cdr=PairV(NullV(),NullV());
            ptr=dynamic_cast<Pair*>(ptr->cdr.get());
            ptr->car=*iter;
        }
        return v;
    }
}

Value IsList::evalRator(const Value &rand) { // list?
    if(rand->v_type==V_NULL){return BooleanV(true);}
    else if(rand->v_type!=V_PAIR){return BooleanV(false);}
    else{
        auto ptr=dynamic_cast<Pair*>(rand.get());
        while(ptr->cdr->v_type==V_PAIR){
            ptr=dynamic_cast<Pair*>(ptr->cdr.get());
        }
        if(ptr->cdr->v_type==V_NULL){
            return BooleanV(true);
        }else{return BooleanV(false);}
    }
}

Value Car::evalRator(const Value &rand) { // car
    if(rand->v_type!=V_PAIR){
        throw RuntimeError("car: expected pair");
    }
    else{
        auto ptr=dynamic_cast<Pair*>(rand.get());
        return ptr->car;
    }
}

Value Cdr::evalRator(const Value &rand) { // cdr
    if(rand->v_type!=V_PAIR){
        throw RuntimeError("cdr: expected pair");
    }
    else{
        auto ptr=dynamic_cast<Pair*>(rand.get());
        return ptr->cdr;
    }
}

Value SetCar::evalRator(const Value &rand1, const Value &rand2) { // set-car!
    if(rand1->v_type!=V_PAIR){throw RuntimeError("SetCar!:required pair");}
    auto ptr=dynamic_cast<Pair*>(rand1.get());
    if(ptr==nullptr){throw RuntimeError("SetCar!:Invalid Pair");}
    ptr->car=rand2;
    return VoidV();
}

Value SetCdr::evalRator(const Value &rand1, const Value &rand2) { // set-cdr!
   if(rand1->v_type!=V_PAIR){throw RuntimeError("SetCdr!:required pair");}
    auto ptr=dynamic_cast<Pair*>(rand1.get());
    if(ptr==nullptr){throw RuntimeError("SetCdr!:Invalid Pair");}
    ptr->cdr=rand2;
    return VoidV();
}

Value IsEq::evalRator(const Value &rand1, const Value &rand2) { // eq?
    // 检查类型是否为 Integer
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        return BooleanV((dynamic_cast<Integer*>(rand1.get())->n) == (dynamic_cast<Integer*>(rand2.get())->n));
    }
    // 检查类型是否为 Boolean
    else if (rand1->v_type == V_BOOL && rand2->v_type == V_BOOL) {
        return BooleanV((dynamic_cast<Boolean*>(rand1.get())->b) == (dynamic_cast<Boolean*>(rand2.get())->b));
    }
    // 检查类型是否为 Symbol
    else if (rand1->v_type == V_SYM && rand2->v_type == V_SYM) {
        return BooleanV((dynamic_cast<Symbol*>(rand1.get())->s) == (dynamic_cast<Symbol*>(rand2.get())->s));
    }
    // 检查类型是否为 Null 或 Void
    else if ((rand1->v_type == V_NULL && rand2->v_type == V_NULL) ||
             (rand1->v_type == V_VOID && rand2->v_type == V_VOID)) {
        return BooleanV(true);
    } else {
        return BooleanV(rand1.get() == rand2.get());
    }
}

Value IsBoolean::evalRator(const Value &rand) { // boolean?
    return BooleanV(rand->v_type == V_BOOL);
}

Value IsFixnum::evalRator(const Value &rand) { // number?
    return BooleanV(rand->v_type == V_INT);
}

Value IsNull::evalRator(const Value &rand) { // null?
    return BooleanV(rand->v_type == V_NULL);
}

Value IsPair::evalRator(const Value &rand) { // pair?
    return BooleanV(rand->v_type == V_PAIR);
}

Value IsProcedure::evalRator(const Value &rand) { // procedure?
    return BooleanV(rand->v_type == V_PROC);
}

Value IsSymbol::evalRator(const Value &rand) { // symbol?
    return BooleanV(rand->v_type == V_SYM);
}

Value IsString::evalRator(const Value &rand) { // string?
    return BooleanV(rand->v_type == V_STRING);
}

Value Begin::eval(Assoc &e) {
    if (es.empty()) {
        return VoidV();
    }
    Value result = VoidV();
    for (auto& expr : es) {
        result = expr->eval(e);
    }
    return result;
}

Value Quote::eval(Assoc& e) {
    if (auto num = dynamic_cast<Number*>(s.get())) {
        return IntegerV(num->n);
    }
    else if (auto rational = dynamic_cast<RationalSyntax*>(s.get())) {
        return RationalV(rational->numerator, rational->denominator);
    }
    else if (auto sym = dynamic_cast<SymbolSyntax*>(s.get())) {
        return SymbolV(sym->s);
    }
    else if (auto str = dynamic_cast<StringSyntax*>(s.get())) {
        return StringV(str->s);
    }
    else if (dynamic_cast<TrueSyntax*>(s.get())) {
        return BooleanV(true);
    }
    else if (dynamic_cast<FalseSyntax*>(s.get())) {
        return BooleanV(false);
    }
    else if (auto list = dynamic_cast<List*>(s.get())) {
        if (list->stxs.empty()) {
            return NullV();
        }
        Value current = NullV();
        for (int i = list->stxs.size() - 1; i >= 0; --i) {
            Quote quote_expr(list->stxs[i]);
            Value element = quote_expr.eval(e);
            current = PairV(element, current);
        }
        return current;
    }
    throw RuntimeError("Unsupported syntax in quote");
}

Value AndVar::eval(Assoc &e) {
    if(rands.empty()){return BooleanV(true);} // and with short-circuit evaluation
    for(auto expr:rands){
        Value value=expr->eval(e);
        auto ptr=dynamic_cast<Boolean*>(value.get());
        if(ptr!=nullptr){
            if(ptr->b==false){return BooleanV(false);}
        }
    }
    return rands.back()->eval(e);
}

Value OrVar::eval(Assoc &e) {
    if(rands.empty()){return BooleanV(false);} // or with short-circuit evaluation
    for(auto expr:rands){
        Value value=expr->eval(e);
        auto ptr=dynamic_cast<Boolean*>(value.get());
        if(ptr==nullptr){return value;}
        else if(ptr!=nullptr){
            if(ptr->b!=false){return value;}
        }
    }
    return rands.back()->eval(e);
}

Value Not::evalRator(const Value &rand) { // not
    auto ptr=dynamic_cast<Boolean*>(rand.get());
    if(ptr==nullptr){return BooleanV(false);}
    else if(ptr->b==false){return BooleanV(true);}
    else if(ptr->b==true){return BooleanV(false);}
}

Value If::eval(Assoc &e) {
    Value condValue=cond->eval(e);
    auto ptr=dynamic_cast<Boolean*>(condValue.get());
    if(ptr==nullptr){return conseq->eval(e);}
    else{
        if(ptr->b==true){return conseq->eval(e);}
        else if(ptr->b==false){return alter->eval(e);}
    }
}

Value Cond::eval(Assoc &env) {
    if(clauses.empty()){return VoidV();}
    for(int j=0;j<clauses.size()-1;++j){
        if(clauses[j].empty()){continue;}
        for(int i=0;i<clauses[j].size()-1;++i){
            Value val=clauses[j][i]->eval(env);
            auto ptr=dynamic_cast<Boolean*>(val.get());
            if(ptr==nullptr||ptr->b==true){
                return clauses[j].back()->eval(env);
            }
        }
    }
    return clauses.back().back()->eval(env);
}

Value Lambda::eval(Assoc &env) { 
    return ProcedureV(x,e,env);
}//只负责捕获，不修改当前环境

Value Apply::eval(Assoc &env) {
    Value proc_value = rator->eval(env);
    if (proc_value->v_type != V_PROC) {
        throw RuntimeError("Attempt to apply a non-procedure");
    }
    Procedure* proc = dynamic_cast<Procedure*>(proc_value.get());
    if (proc==nullptr) {
        throw RuntimeError("Invalid procedure object");
    }
    std::vector<Value> args;
    for (auto &arg_expr : rand) {
        args.push_back(arg_expr->eval(env));
    }
    if (args.size() != proc->parameters.size()) {
        throw RuntimeError("Wrong number of arguments");
    }
    Assoc new_env = proc->env;  
    for (size_t i = 0; i < args.size(); ++i) {
        new_env = extend(proc->parameters[i], args[i], new_env);
    }
    return proc->e->eval(new_env);
}

Value Define::eval(Assoc &env) {
    if (primitives.count(var)) {
        throw RuntimeError("Cannot redefine primitive: " + var);
    }
    if (reserved_words.count(var)) {
        throw RuntimeError("Cannot use reserved word as variable: " + var);
    }
    Value value = e->eval(env);
    extend(var, value, env);
    return VoidV();
}

Value Let::eval(Assoc &env) {
    Assoc new_env = env;
    for (auto& binding : bind) {
        Value value = binding.second->eval(env);
        new_env = extend(binding.first, value, new_env);
    }
    return body->eval(new_env);
}

Value Letrec::eval(Assoc &env) {
    if (bind.empty()) {
        return body->eval(env);
    }
    Assoc new_env = env;
    for (auto& binding : bind) {
        new_env = extend(binding.first, VoidV(), new_env);
    }
    for (auto& binding : bind) {
        Value value = binding.second->eval(new_env);
        modify(binding.first, value, new_env);
    }
    return body->eval(new_env);
}

Value Set::eval(Assoc &env) {
    try {
        find(var, env);
    } catch (const RuntimeError&) {
        throw RuntimeError("Variable not found: " + var);
    }
    Value new_value = e->eval(env);
    modify(var, new_value, env);
    return new_value; 
}


Value Display::evalRator(const Value &rand) { // display function
    if (rand->v_type == V_STRING) {
        String* str_ptr = dynamic_cast<String*>(rand.get());
        std::cout << str_ptr->s;
    } else {
        rand->show(std::cout);
    }
    
    return VoidV();
}
