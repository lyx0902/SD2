# 南京理工大学软件课程设计(II)——2025

**计算机科学与技术22级刘宇翔**

**王永利老师**

### 课设答辩问题

#### 1.词法分析中，把复数拆开加个乘号他应该怎么识别（变成算术表达式，拆开识别）

**例如2.0-3.0i**

**识别出来是**

(Line: 5, Type: Constant, Value: 2.0-3.0i)

拆开后要为**2.0 - 3.0 * i**

**识别出来是**

(Line: 5, Type: Constant, Value: 2.0)

(Line: 5, Type: Operator, Value: -)

(Line: 5, Type: Constant, Value: 3.0)

(Line: 5, Type: Operator, Value: *)

(Line: 5, Type: Identifier, Value: i)

注意在本代码中是以空格为分隔符，故而之于正常的写代码习惯有一定区别，留给后人进行todo

#### 2.LR(1)语法分析怎么判断移进归约冲突

找到对应位置的代码，说明函数的功能，给出处理的是优先移进还是规约判断

#### 3.说一下dfa nfa和action goto表的数据结构

找到对应的头文件数据结构声明，对照输出的结果看一下数据结构中都存入了哪些类型的属性，怎么实现

