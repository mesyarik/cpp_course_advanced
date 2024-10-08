
## 3.1. Classes and structures (классы и структуры)  

В  этом  разделе мы начнем изучать объектно-ориентированное программирование (ООП). По сути, всю оставшуюся часть курса мы будем заниматься именно ООП. Сейчас ООП – это покорившая мир парадигма программирования, почти все современные популярные языки являются объектно-ориентированными. В разных языках основные понятия и принципы ООП одинаковы, отличаются только детали.

Мы уже могли заметить, что любая программа – это, по сути, создание каких-то переменных и выполнение операций над ними, после чего переменные уничтожаются и программа завершается. У каждой переменной обязательно должен быть свой тип. Тип как раз определяет то, какие операции можно выполнять над данной переменной. ООП позволяет нам создавать собственные типы и определять операции над ними. Двумя, пожалуй, главными понятиями ООП являются **классы** и **объекты**. Классы в С++ - это как раз пользовательские типы данных (но не любые, мы уточним это позже). Объекты – это сущности, типом которых является какой-либо класс. Впрочем, в C++ формально объектами считаются и сущности примитивных типов (вероятно, для удобства формулировок).

Существует три основных принципа ООП. Они называются **инкапсуляция, наследование и полиморфизм**. Мы по очереди подробно рассмотрим их все в этом курсе.  

Вот простейший пример определения класса:

```cpp
class C {
};

int main() {
  C c;
}
```
Здесь class C {}; - это определение класса с именем C. После того как он определен, можно использовать его в коде как тип переменных. В данном случае мы создали в main переменную c, которая имеет тип C. После определения класса (в отличие от определения функции) необходимо ставить точку с запятой. Напомню, что класс, как и функцию, можно объявить без определения, а определить когда-нибудь потом. Или вообще не определять, но тогда нельзя будет создавать переменные такого типа.  

```cpp
class C;  
int main() {  
  C c; // CE! Class C was declared but not defined  
}
```
  
Кроме классов, можно объявлять еще структуры, для этого вместо слова class надо писать слово struct. В С++ структуры почти ничем не отличаются от классов, за исключением пары мелочей, о которых будет сказано ниже. Во всем дальнейшем изложении я буду использовать слова “класс” и “структура” как синонимы, если явно не сказано обратное.

Когда мы объявили класс и открыли фигурную скобку, начинающую его определение, то мы оказываемся в области видимости, которая называется **class scope.** В ней, как и в глобальной области, как и в namespace scope, можно писать только другие объявления (но не выражения и не управляющие конструкции). Например, вот это было бы ошибкой:

```
class C {  
  int x;
  std::cin >> x; // CE! Expression is not allowed here
};
```
Переменные, объявленные в классе, называются **полями (fields)** этого класса. Функции, объявленные в классе, называются **методами (methods)** этого класса. Разумеется, если какая-то переменная объявлена в теле метода, то она уже полем класса не является, она является локальной переменной для этого метода.  

Поля – это данные, которые будет содержать объект этого класса. Синтаксис объявления полей в классе такой же, как и синтаксис объявления обычных локальных переменных в функциях. Методы – это операции, которые будут поддерживаться у объектов данного класса. Синтаксис определения методов в классе такой же, как и синтаксис определения обычных функций. Поля и методы класса совместно называются **членами (members)** этого класса.

Вот пример (здесь я использую struct вместо class по причине, о которой скажу ниже):

```cpp
struct S {
  int x;
  std::string s;
  double d;
  
  void f(int a) {
    std::cout << x + a << std::endl;
  }
};
```  

В данной структуре определены поля x, s, d и метод f(int). В теле метода f можно оперировать и полями класса (включая x), и локальной переменной a. Если бы параметр у метода назывался так же, как поле (скажем, x), то по общим правилам предпочтение бы отдавалось более локальному имени, и внутри f имя x означало бы локальную переменную, а не поле класса.

Если имеется объект класса, то для обращения к его членам используется **оператор точка**. Пусть структура S определена как написано выше, тогда мы можем, например, написать такой код:  

```cpp
int main() {
  S s;
  std::cout << s.s.size() << s.d << std::endl;
  s.s = "abc";
  s.f(5);
}
```

При запуске этого кода мы увидим сначала 0, а затем рандомные числа. Дело в том, что мы ничем не проинициализировали поля класса по умолчанию, поэтому поведение такое же, как если бы мы объявили аналогичные переменные локально в некоторой функции. Для типов int и double это означает, что там будут лежать рандомные числа, но для std::string гарантируется, что будет создана пустая строка.

Предположим теперь, что у нас имеется не объект некоторого класса сам по себе, а указатель на него. Например, такое могло произойти, если мы создали объект типа S в динамической памяти и тем самым получили переменную типа S*:  
  
```cpp
S* ps = new S;
```
Чтобы обратиться, например, к полю x только что созданного объекта, мы могли бы написать *ps.x. Но оператор точка имеет более высокий приоритет, чем разыменование указателя. Поэтому нам нужны будут скобки: (*ps).x. Чтобы обращаться к полям объекта, имея лишь указатель на этот объект, было удобнее, существует **оператор стрелочка (arrow operator)**. Он записывается как ->. Чтобы теперь обратиться к полю x, имея указатель ps, можно написать: ps->x. Это эквивалентно записи (*ps).x.

Точка и стрелочка имеют такой же приоритет, как и постфиксные унарные операторы, и выше, чем префиксные унарные. Более высокий приоритет имеет только двойное двоеточие ::. Применение точки или стрелочки к переменным, не являющимся объектами, -- это бессмыслица и CE.  

Если мы хотим, чтобы при создании объекта структуры ее поля по умолчанию инициализировались какими-то конкретными значениями, а не рандомными числами, то мы можем добавить полям инициализаторы по умолчанию:

```cpp
struct S {
  int x = 5;
  std::string s = "abc";
  double d = 0.0;
};
```

Теперь, если мы в main создадим объект S s; и выведем значения его полей s.x и s.d, то мы гарантированно увидим эти значения. А поле s.s теперь будет строкой из 3 символов, а не пустой строкой.

Мы можем, однако, указать желаемые значения полей прямо при создании структуры. Для этого можно использовать так называемую **агрегатную инициализацию (aggregate initialization)**. Делается это так:  

```cpp
int main() {
  S s{1, "abcde", 3.14};
  std::cout << s.x << s.s << s.d;
}
```
В этом примере поля изначально приобретут такие значения, какие мы указали в фигурных скобках. Порядок, в котором мы перечисляем значения полей при агрегатной инициализации, должен строго соответствовать порядку, в котором поля объявлены в структуре. Если мы, например, написали бы так:
```cpp
S s{1, 3.14, "abcde"};
```
-- то получили бы CE, поскольку невозможно проинициализировать std::string значением 3.14.  

Мы также получим CE, если укажем большее количество значений, чем количество полей в структуре. А вот если укажем меньшее количество, то ошибки не будет, просто оставшимся полям дадутся значения по умолчанию (будут применены инициализаторы по умолчанию, если они есть).

Агрегатная инициализация применима далеко не ко всем классам и структурам, но условия ее применимости мы обсудим позже.

Вернемся к разговору о методах класса. Выше был показан пример определения метода f в теле структуры S. Но на самом деле можно определять метод и вне тела класса, при условии, что он объявлен в теле класса. Вот как это будет выглядеть:

 ```cpp
struct S {
  int x;
  void f(int a);
};
  
void S::f(int a) {
  std::cout << x+a;
}
```
  

Чтобы определить метод вне класса, мы используем квалифицированное имя S::f вместо обычного f. Этим мы указываем компилятору, что определяем именно f из класса S, а не просто внешнюю f. Обратите внимание на этот синтаксис: S:: ставится перед именем функции, а не перед возвращаемым типом (имя f берется из класса S, а вовсе не тип void)! Но если мы попытаемся так сделать, предварительно не объявив f с такой сигнатурой в теле S, то, разумеется, получим CE.

Как и в обычных функциях, названия принимаемых аргументов при объявлении и при определении методов не обязаны совпадать (я мог бы написать f(int b) вместо f(int a)). Кроме того, в объявлении можно вообще не писать имя аргумента, а если он не используется в теле функции -- то его можно не писать и в определении (то есть писать просто f(int)). Заметьте, что в теле метода я использую поле x, несмотря на то, что метод определяю вне класса -- это нормально и не является ошибкой.

Методы классов, как и обычные функции, можно перегружать. То есть мы могли бы определить еще метод f, например, от double. Тогда при выборе версии метода применялись бы обычные правила разрешения перегрузки.

Выше я говорил, что если имя у параметра метода такое же, как имя некоторого поля, то в теле метода предпочтение будет отдаваться более локальному имени:  

```cpp
struct S {
  int x = 1;
  void f(int x) {
    std::cout << x;
  }
};

int main() {
  S s;
  s.f(2); // prints 2, not 1
}
```
Однако существует способ из тела метода явно сказать, что мы хотим обратиться к нашему полю, а не к локальной переменной. Для этого существует **ключевое слово this.**

Если мы находимся в теле метода структуры S, то this само по себе -- это выражение, имеющее тип S*, а значение this равно адресу текущего объекта. Проще говоря, this -- это “указатель на себя”. В языке Python аналогичную роль выполняет слово self. Использование слова this вне метода класса -- это бессмыслица и CE.

Таким образом, чтобы обратиться к полю x вместо локальной переменной, мы можем написать:  

```cpp
struct S {
  int x = 1;
  void f(int x) {
    std::cout << this->x;
  }
};
```
Если коллизий имен нет, то писать this для обращения из методов к полям и другим методам этого же класса не нужно, это избыточно. По кодстайлу не рекомендуется писать this, когда это не необходимо (другие случаи, кроме коллизии имен, когда это нужно, мы увидим позже).

В начале этого параграфа я говорил, что в теле классов можно делать только объявления. Мы разобрали объявления полей и методов. Однако можно делать и using-объявления, например:  

```cpp
struct S {
  using MyType = std::vector<int>;
};
```
 
-- после такого объявления в теле этого класса (включая тела методов) MyType будет означать std::vector<int>.

Определять namespace внутри классов нельзя. Но внутри классов можно объявлять и определять другие классы. Тогда они называются **вложенными классами (inner classes)**. Рассмотрим такой код для примера:

```cpp
struct S {
  int x = 1;
  double y = 3.14;
  
  struct SS {
    float f = 1.05;
  };
};
```
 
Здесь надо понимать, что SS -- это не поле класса S. Это другой класс, область видимости которого ограничена классом S. Мы могли бы создать в классе S поле типа SS, если бы после определения SS написали, например, SS ss; . Но в примере выше мы не создали ни одного объекта типа SS. Полями S являются только x и y. Мы можем использовать тип SS, например, в методах класса S: 

```cpp
struct S {
  struct SS {
    float x = 1.05;
  };
  void f() {
    SS ss;
    std::cout << ss.x;
  }
};

int main() {
  S s;
  s.f(); // outputs 1.05
}
```
  
Внутренние классы для S также можно определять вне S, при условии, что они объявлены внутри S. Тогда при определении класса SS нам пришлось бы использовать квалифицированное имя: struct S::SS.  

Тот факт, что класс SS является внутренним для S, не запрещает использовать SS вне S. Более того, для существования объектов типа SS не нужно существование объектов типа S. Но чтобы использовать SS вне S, нужно указывать квалифицированное имя, то есть уточнять, что это SS из S, а не из глобальной области видимости. Например, так:
  
```cpp
int main() {
  S::SS ss{3.14};
  std::cout << ss.x; // OK, outputs 3.14
}
```
  
Также замечу, что из тела SS нельзя использовать поля S напрямую:

```cpp
struct S {
  int x = 1;
  struct SS {
    int y = x; // CE! We don’t have any object of type S
  };
};
```
  
Надо понимать, что SS и S -- это разные, никак не связанные друг с другом типы (кроме того, что определение SS находится в области видимости внутри S). Когда мы находимся в теле SS, у нас есть лишь наши собственные поля. Поля внешнего класса для нас полями не являются, и обращение к ним осмысленно только как к полям некоторого объекта типа S. Аналогично, из методов SS мы не можем напрямую оперировать полями S, потому что это бессмыслица: у нас нет никакого объекта типа S, чтобы иметь дело с его полями.

  

Определение классов внутри других классов -- это весьма распространенное явление в реальном коде. Такое распространено и в стандартной библиотеке: например, таковым является тип std::vector\<int\>::iterator. Здесь iterator -- это, по сути, класс, определенный внутри класса std::vector\<int\>. Нам еще предстоит подробный разговор о том, что такое iterator и как он устроен изнутри.


Можно объявлять и определять классы внутри функций. Тогда они называются **локальными классами (local classes)**. Например:
 
```cpp
int main() {
  struct S {
    int x = 1;
    void f(int a) {
      std::cout << x+a;
    }
  };
  S s;
  s.f(2); // OK, outputs 3
}
```
  

В такой ситуации тип S можно использовать только изнутри функции main. Локальные классы -- не так часто встречающееся явление, как вложенные классы, но тем не менее иногда полезное.

Напоследок поговорим о том, как объекты классов располагаются в памяти. Рассмотрим такую структуру:

```cpp
struct S {
  int x = 1;
  char c = 'a';
  double d = 3.14;
};
```

TODO размер структур, пример изменения размера от перестановки полей
