# Project 3: Scheme Interpreter
> SJTU CS1958-01 2025Fall 第三次大作业
在完成本次大作业的过程中，你可以修改任何相关的代码，比如当你处理 `VoidV` 时，很可能会涉及对 `main.cpp` 的修改.
+ ;;Value: + 作为 primitive，是一个函数
;;Value: #<procedure>
((if #t + -) 1 2) ;;Value: (if #t + -) 的值为 +，然后调用进行函数调用
;;Value: 3
```
//这里有问题
```Scheme
(((lambda (+) +) *) 2 3)
;;Value: 6
注意这里的 `procedure` 如果包含多个表达式，你需要从左到右依次执行.
//怀疑是apply的问题
3. 对一元简单递归函数的处理; 为此，你需要先在环境中创建一个占位符绑定，然后计算表达式的值，最后将更新绑定为实际值
```Scheme
(define (fact n)
  (if (= n 1)
      1
      (* n (fact (- n 1)))))
```
//这里有问题
递归在 `Scheme` 中常用来表示重复；表是被递归定义的，进而表和递归函数可以很好地配合。例如，一个让表中所有元素翻倍的函数可以像下面这样写。如果参数是空表，那么函数应该停止计算并返回一个空表。
```Scheme
(define (list*2 ls)
  (if (null? ls)
      '()
      (cons (* 2 (car ls))
             (list*2 (cdr ls)))))
```

上述过程中，我们保证所有的 `define` 都在全局环境中；
不会出现 `define` 内嵌套在 `begin`,`define`,`let` 等可能出现创建局部环境的情况.
注意这里的第二个参数如果包含多个表达式，你需要从左到右依次执行.
`let` 表达式可以嵌套使用。
例 4: 这里的变量绑定与 `var` 要求相同，支持 `primitives` 与 `reserved_words`;
(let ((+ -)) (+ 2 1))
;;Value: 1
#### 嵌套定义的实现
对于一元递归的实现是简单，现在我们希望你能够实现形如下文的嵌套定义:
```Scheme
(define (is-even n)
  (cond
   ((= n 0) #t)
   (else (is-odd (- n 1)))))
(define (is-odd n)
  (cond
   ((= n 0) #f)
   (else (is-even (- n 1)))))
```
#### 词法闭包的实现
你可以使用词法闭包来实现带有内部状态的过程。例如，用于模拟银行账户的过程可以按如下的方式编写：初始资金是 `10` 美元。函数接收一个整形参数。正数表示存入，负数表示取出。为了简单起见，这里允许存款为负数。
```Scheme
(define bank-account
  (let ((balance 10))
    (lambda (n)
      (set! balance (+ balance n))
      balance)))
```
该过程将存款赋值为 `(+ balance n)`。下面是调用这个过程的结果。

```Scheme
(bank-account 20)     ; donating 20 dollars 
;Value: 30
(bank-account -25)     ; withdrawing 25 dollars
;Value: 5
```                         
因为在 `Scheme` 中，你可以编写返回过程的过程，因此你可以编写一个创建银行账户的函数。这个例子喻示着使用函数式程序设计语言可以很容易实现面向对象程序设计语言。实际上，只需要在这个基础上再加一点东西就可以实现一门面向对象程序设计语言了。

```Scheme
(define (make-bank-account balance)
  (lambda (n)
    (set! balance (+ balance n))
    balance))
(define gates-bank-account (make-bank-account 10))   ; Gates makes a bank account by donating  10 dollars
;Value: gates-bank-account

(gates-bank-account 50)                              ; donating 50 dollars
;Value: 60

(gates-bank-account -55)                             ; withdrawing 55 dollars
;Value: 5


(define torvalds-bank-account (make-bank-account 100))  ; Torvalds makes a bank account by donating 100 dollars
;Value: torvalds-bank-account

(torvalds-bank-account -70)                             ; withdrawing 70 dollars
;Value: 30

(torvalds-bank-account 300)                             ; donating 300 dollars
;Value: 330
```

