Напишете програма parallel.c, която приема един аргумент (число N) - максимален брой
паралелни задачи.

На stdin програмата получава задачи, разделени с нов ред.

Простите задачи представляват просто команда с аргументи.

Може да строим и по-сложни задачи с операторите ";" и "|":
  "<команда1> ; <команда2>" означава командите да се изпълнят последователно
  "<команда1> | <команда2>" означава командите да се изпълнят
  едновременно, като stdout на команда1 е пренасочен към stdin на команда2.

  Операторът ";" е с по-нисък приоритет от оператора "|".

  Аргументите на командите се разделят помежду си с един или
  повече whitespace символа, а операторите - с нула или повече.
  Т.е. следното е валидна задача:

  `   cat foo  | grep   -F bar;echo baz  `

  Децата на задачата, породени от пайповете не се броят за отделни задачи.

  `cat foo | grep -F bar ; echo baz` се интерпретира като "изпълни cat foo и
  grep -F bar, пренасочвайки изхода на първото във входа на второто, изчакай
  ги да приключат и изпълни echo baz"

  Приемете, че между всеки два оператора ";" има най-много един "|". (*)

Приемаме, че аргументите се състоят само от букви, цифри и долни черти (т.е.
не трябва да поддържаме кавички).

Целта е да изпълни всички задачи максимално бързо, т.е. да се стреми във всеки
момент да има N на брой паралелни задачи, освен когато няма повече задачи за
изпълнение.

Основният процес не трябва да терминира преди да са завършени всички задачи.

Програмата може да изписва ориентировъчен текст по ваш избор на stderr, а на
stdout трябва да излиза текста, изведен на stdout от задачите. stderr-изходът
на задачите може да е смесен с stderr-изхода на вашата програма.
(няма проблем изходите на задачите да са смесени помежду си заради паралелното
изпълнение).

Ето пример:

$ cat tasks.txt
sleep 2 ; echo second
sleep 4 ; echo fourth | sed s:four:4:g
echo f1rst
sleep 1 ; echo third

$ ./parallel 2 < tasks.txt
second
f1rst
third
4th

Обяснение на примера:
* Малко след секунда 0:
Задачите, които изписват second и fourth започват да се изпълняват едновременно.
Останалите чакат, защото имаме право само на 2 паралелни.
* Малко след секунда 2:
Изписваме second и задачата завършва. Започваме да изпълняваме задачата, която
изписва f1rst. Тя завършва веднага. Започваме да изпълняваме задачата, която
изписва third.
* Малко след секунда 3:
Изписваме third и задачата завършва. Няма повече задачи за изпълнение, т.е. в
момента само чакаме да завърши последната задача.
* Малко след секунда 4:
Изписваме 4th и приключваме.

Бонус:
  - Игнорирайте (*) - т.е. програмата ви да позволява произволен брой пайпове
    един след друг.

    Подсказка: най-лесно е да направите функции за "извикване на проста команда"
    и "извикване на две пайпнати подзадачи" и "извикване на произволна задача",
    които се викат рекурсивно - така обработката на произволен брой пайпове
    ще ви излезе безплатно.

За тези от вас, които предпочитат формални описания, приемете, че задачите имат
вид <task>, където имате следната граматика (заедно с бонусите):

task    ::= <command>
task    ::= <command>|<task>
task    ::= <command>;<task>

command     ::= <callable>( <argument>)*

callable    ::= <token>

argument    ::= <token>

token       ::= [a-zA-Z0-9_/]+

Тук приемаме, че преди и след всеки нетерминал (<foo>) може да има произволен брой
whitespace-символи.
