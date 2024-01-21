libles - библиотека для работы с базами знаний Малой экспертной системы 2.0
===========================================================================

Данная библиотека предназначена для работы с базами знаний, созданными в
устаревшей [Малой экспертной системе](http://bourabai.ru/alg/mes2.htm), которая
была разработана неким Алексеем Бухниным и выпущена под проприетарной лицензией.
Программа устарела, плохо открывается в Windows 11, справка хранится в давно
неподдерживаемом формате `.HLP`, на неё ругаются антивирусы. 

Данная библиотека ставит своей целью поддерживать формат базы знаний, слава
богу, описанный на странице Малой экспертной системы, а также проводить
тестирования и формировать протоколы, совместимые с Малой экспертной системой.

**Планируется выпустить данную библиотеку под свободной лицензией с сильным
копилефтом - GNU GPL v3.**

Использование библиотеки
------------------------

Библиотеку можно использовать тремя основными способами:

- как динамическую (то есть, с указанием `-D_LES_DLL` в флагах компилятора);

- как статическую;

- в качестве исходных файлов напрямую.

Библиотеку можно использовать в соответствии с условиями лицензии GNU GPL v3.

Используемые библиотеки
-----------------------

В данной библиотеке используется код других библиотек в виде исходников. Они
подобраны таким образом, чтобы спрятать особенности их использования от
программиста и быть максимально совместимыми со стандартными интерфейсами
разработки приложений в Си.

Используются следующие библиотеки:

1. buf.h (<https://github.com/skeeto/growable-buf>) - общественное достояние;

Инструкция по компиляции
------------------------

На сегодняшний день поддерживается компиляция на Windows и Linux. На Windows
компиляция осуществляется с использованием mingw-w64 (используется w64devkit).

Для компиляции достаточно набрать `make` в командной строке. В итоге
скомпилируется:

- динамическая библиотека `libles.so` (на Windows - `libles.dll` вместе с
  импортной библиотекой `libles.lib`);

- статическая библиотека `libles.a` (на Windows - `libless.lib`);

- приложение `lesrun.exe`, использующее динамическую библиотеку `libles.dll`;

- приложение `lesruns.exe`, использующее статическую библиотеку `libless.lib`.

Формат файла базы знаний
------------------------

*Оригинальное описание доступно по адресу <http://bourabai.ru/alg/mes2.htm>*

База знаний представляет собой текстовый файл (который в дальнейшем может быть
зашифрован), включающий три секции со следующей структурой:

1. Описание базы знаний, имя автора, комментарий и т.п.

   Можно в несколько строк, общая длина которых не должна превышать 10000
   символов. Данная секция заканчивается после первой пустой строки.

2. Свидетельства.

   Свидетельство №0 (любой текст не более 1000 символов, заканчивающийся
   переносом строки)

   Свидетельство №1

   Свидетельство №2

   ...

   Свидетельство №N (после последнего свидетельства следует одна пустая строка,
   и вторая секция заканчивается).

3. Исходы

   Исход №0, P [, i, Py, Pn]

   Исход №1, P [, i, Py, Pn]

   Исход №2, P [, i, Py, Pn]

   ...

   Исход №M, P [, i, Py, Pn]


TODO
----

- [X] Немного исправить формат хранения данных о вероятностях "да/нет", чтобы
  было удобнее искать их потом

- [X] Добавить модуль expert, который будет задавать вопросы и принимать ответы
  на них

- [X] Добавить поддержку разных кодировок - CP1251, UTF-8, ~UTF-16LE~. Здесь мы
  исходим из предположения, что оригинальная Малая экспертная система создаёт
  файлы исключительно в однобайтной кодировке CP1251.

  Кодировка UTF-16LE не планируется.

- [ ] Разработать на базе какого-нибудь XML, JSON или тому подобного новый
  формат хранения данных (или не надо?)

- [X] Добавить компиляцию в качестве динамической библиотеки (DLL, SO)

- [X] Добавить компиляцию в качестве статической библиотеки (LIB, A)

- [ ] Прикрутить к этой библиотеке графическую морду для Windows

- [ ] Добавить поддержку C89 и компилятора Microsoft Visual C++

Дальнейшие перспективы
----------------------

1. Библиотеку можно портировать на другие платформы, например, на JVM (Java,
   Kotlin и т.д.), .NET (C#, Visual Basic), если кому надо

2. Библиотеку можно портировать на другие языки программирования, например, на
   Go, Rust, Pascal, если кому надо
