Решения к задачам
=================

.. _custom-main:

Custom main
-----------

Сразу перейдём к одному из *возможных* решений:

.. include:: custom_main.cpp
    :code: cpp

По умолчанию все программы линкуются статически с объектным файлом ``crt0.o``, который содержит настоящую точку входа в программу ``_start``.

Функция ``__libc_start_main`` вызывается кодом crt0 из динамически слинкованной библиотеки ``libc``.
Её работа заключается в инициализации программы, в частности в вызове конструкторов статических объектов, в том числе так называемых *статических конструкторов*.
Как правило, ``libc`` всегда линкуется динамически: т.е., одна библиотека в системе обслуживает все исполняемые программы.
Этим мы и воспользуемся.
Опишем все тонкости реализации программы по порядку.

Hiding символов при динамической линковке
    На этапе линковки программы важен порядок передаваемых динамических библиотек.
    При наличии одинаковых символов (в нашем случае это ``__libc_start_main``) во время исполнения программы будет вызван тот, который встретится раньше.
    Таким образом будет вызвана наша функция ``__libc_start_main``, а не аналогичная из библиотеки ``libc``.
    Заметим, что если библиотека будет линковаться статически, то этот фокус не будет работать.
    Существует способ, при котором можно попросить загрузчика динамических библиотек ``ld`` вернуть адрес следующего символа с таким же названием.
    Для этого служит флаг ``RTLD_NEXT``, с помощью которого можно получить все функции с одинаковыми именами в динамически слинкованных с нашей программой библиотеках.
    Поэтому мы просто сделаем необходимые нам действия, а потом вызовем настоящую ``__libc_start_main``.
    Чем-то напоминает вызов базовой реализации при наследовании.

Mangling имён
    Язык C++ добавляет специальные префиксы и суффиксы к именам функций, чтобы поддерживать такие технологии, как перегрузку функций и пространства имён.
    Язык C же в свою очередь добавляет только префикс: дополнительный ``_`` к имени функции.
    Чтобы C++ правильно сформировал имя функции ``__libc_start_main``, необходимо сказать ему "делай как в языке C".
    Для этого мы добавляем модификатор ``extern "C"``.

Статические переменные
    Вектор с аргументами объявлен с модификатором ``static``.
    Это необходимо, так как некоторые реализации ``__libc_start_main`` не вызывают напрямую функцию ``main``, а оставляют это для следующего кода.
    Чтобы вектор не был очищен после выхода из функции, мы объявляем его статическим (все статические объекты - глобальные, с ограниченной областью видимости).
    Почему внутри функции, а не в глобальной области?
    Чтобы не засорять глобальную область.
    Инкапсуляция, однако.

Три аргумента у ``main``
    Самые зоркие уже заметили, что у функции ``main`` три аргумента, а не два.
    Третий аргумент - это таблица с переменными окружения.
    Таблица с аргументами командной строки и переменными окружения лежат на самой вершине стека.
    Помещаются они туда самой операционной системой при загрузке исполняемого файла в память.

Размер одной переменной на стеке
    Воспользуемся ещё одним приёмом: при помещении аргумента функции в стек, её размер увеличиваеся до размера регистра.
    Размер регистра таков, что способен поместить в себе указатель на память (как минимум это ``std::size_t``).
    Таким образом, в первый аргумент можно положить адрес нашего вектора, а не ``int``.

.. _loop-linearization:

Линеаризация циклов
-------------------

Наверное, первое, что приходит на ум, это

.. include:: multiloop_simple.cpp
    :code: cpp

Однако задача наша - *полностью* избавиться от вложенных циклов.
Приведённое же решение является попросту синтаксическим сахаром, которое во время компиляции разворачивается во вложенные циклы.

Да, в математике есть процедура, похожая на вложенные циклы.
И это - *декартово произведение*.
Будет, наверное, неожиданно, если мы скажем, что числа от 0 до 799 - это декартово произведение трёх отрезков:
:math:`{[0, 7]} \times {[0, 9]} \times {[0, 9]}`.
Каждая цифра соответствует своему циклу.
Младшая цифра - это самый вложенный цикл, который "бегает" быстрее всех.
Самая старшая цифра - это внешний цикл, самый "медлительный".

Если перевести числа из десятичной системы счисления в, скажем, двоичную, то количество цифр увеличится.
Но ведь таким образом увеличится и количество вложенных циклов!
А если перевести в 16-ричную систему, тогда цифр станет меньше, причём циклов тоже.
Вот оно и решение: чтобы линеаризовать циклы, необходимо использовать системы счисления с большим базисом.

Если с десятичной или любой *N*-ичной системой всё более менее понятно, то как быть, если базис каждой цифры произволен?
Для этого надо вспомнить, как обрабатываются картинки.
Как правило, под картинки выделяется единая линейная область памяти.
Точка с координатами :math:`(x, y)` должна перейти в линейный адрес `i` этой памяти с помощью формулы:

.. math:: i = y \cdot width + x

где `width` - это ширина изображения.
Полное количество точек изображения определяется как :math:`width \cdot height`.

Куб определяется как набор картинок.
Линейный адрес `i` каждой точки куба - это порядковый номер картинки `z`, умноженный на размерность оси :math:`width \cdot height`, плюс номер строки `y`,
умноженный на размерность `width` и номер столбца `x`, размерность которого равна 1:

.. math:: i = z \cdot width \cdot height + y \cdot width + x

Для `N`-мерного куба:

.. math::
    i = x_{n-1} \prod_{k=0}^{n-2} W_k + x_{n-2} \prod_{k=0}^{n-3} W_k + \ldots + x_2 \cdot W_1 \cdot W_0 + x_1 \cdot W_0 + x_0

Как по индексу `i` можно получить координаты в `N`-мерном кубе?

.. [#] Формула динамического базиса
.. math::
    \newcommand\bmod{\mathbin\%}
    \newcommand\trunc[1]{\left[ #1 \right]}
    \begin{aligned}
        x_0 &= i \bmod W_0, \\
        x_1 &= \trunc{\frac{i}{W_0}} \bmod W_1, \\
        x_2 &= \trunc{\frac{i}{W_0 \cdot W_1}} \bmod W_2, \\
        &\ldots, \\
        x_{n-1} &= \trunc{\frac{i}{W_0 \cdot W_1 \cdot \ldots \cdot W_{n-2}}} \bmod W_{n-1},
    \end{aligned}

где :math:`\trunc{\cdot}` - это целая часть числа, а :math:`x \bmod y` - остаток от деления `x` на `y`.
Если произведения в знаменателях :math:`W_0 \cdot W_1 \cdot \ldots \cdot W_{k-1}` заменить на :math:`B^k`, тогда получится формула пересчёта числа `i` в базис `B`.
Здесь и далее будем называть базис, в котором для каждой из цифр выбран свой базис - `динамическим базисом`.

Таким образом, задача сведения вложенных циклов в один - это задача представления числа в динамическом базисе.
Количество цифр - это количество базисов.
Нижняя и верхняя границы каждой цифры определяют размер базиса.

Рассмотрим одно из возможных решений задачи:

.. include:: multiloop.cpp
    :code: cpp

Основная идея заключена в функции ``dereference``.
В ней для каждого ``ranges`` выбирается текущая позиция по формуле динамического базиса (`1`_).
При этом на итераторы ``ranges`` (в качестве них могут выступать и STL контейнеры) наложено ограничение: они должны удовлетворять концепции `RandomAccessIterator`_.

.. _RandomAccessIterator: http://en.cppreference.com/w/cpp/concept/RandomAccessIterator

Кумулятивное умножение ``make_cum_prod`` реализует массив из знаменателей по формуле (`1`_).
Благодаря ему вычисления координат производятся только целочисленным делением и взятием остатка.

Конечно, приведённый вариант линеаризации циклов уступает по производительности первому рекурсивному варианту.
Но порой в некоторых задачах без линеаризации не обойтись.
Ведь мы показали способ, который хорошо подходит не только для линеаризации вложенных циклов, но вообще всех линеаризаций, которые выражаются через динамический базис чисел.

.. _newtons-binom:

Сколько чисел можно составить из 7 единиц и 3 нулей?
----------------------------------------------------

Для точности формулировки: каждое найденной число должно содержать ровно 7 единиц и 3 нуля.
То есть разрядность числа составляет 10 позиций.

Ответ заключается в использовании бинома Ньютона :math:`\newcommand\newtonbinom[2]{C^{#1}_{#2}}\newtonbinom{3}{10}`.

Представим, что все 10 цифр заняты единицами.
Мы будем заменять любые три единицы на символы `a`, `b` и `c`.
`a` может заменить одну из 10 единиц, при этом `b` остаётся 9 позиций, а `c` - всего 8.
В итоге получится, что разместить по 10 позициям `a`, `b` и `c` можно :math:`10 \cdot 9 \cdot 8` различными способами.
А теперь заменим `a`, `b` и `c` на символ 0.
Видим, что некоторые комбинации повторяются, так как все символы нуля одинаковые.
Сколько таких одинаковых комбинаций?

Для ответа на этот вопрос представим отдельно `a`, `b` и `c` по трём позициям.
Снова: `a` может занять 3 позиции, при этом `b` - 2, и `c` - 1.
Получаем, что комбинаций всего будет :math:`3 \cdot 2 \cdot 1 = 3!`.

Вернёмся к изначальной задаче.
Необходимо найти количество комбинаций `x`.
Как мы уже выяснили, всего комбинаций будет :math:`10 \cdot 9 \cdot 8`, но из них :math:`3!` одни и те же.
То есть

.. math:: 10 \cdot 9 \cdot 8 = 3! \cdot x

Откуда следует, что из 7 единиц и 3 нулей можно составить :math:`\frac{10 \cdot 9 \cdot 8}{3!} = 120` чисел.

Разберём два дополнительных вопроса.

Почему :math:`\frac{10 \cdot 9 \cdot 8}{3!}` и :math:`\frac{10!}{7! \cdot 3!}` одно и то же?
    С первой формулой всё понятно: надо вставить 3 нуля в 10 позиций.
    Вторая формула говорит, что необходимо 7 единиц и 3 нуля вставить в 10 позиций.
    По сути, обе формулы означают одно и то же, просто по-разному трактуются.

Сколько чисел можно составить из 7 единиц, 3 нулей, 4 двоек и 2 троек?
    Имея накопленные знания, нетрудно подсчитать, что:

    - всего позиций 16
    - всевозможных перестановок будет :math:`16!`
    - повторяющихся единиц будет :math:`7!`, нулей - :math:`3!`, двоек - :math:`4!`, троек - :math:`2!`

    В итоге получим, что нужное количество чисел составит :math:`\frac{16!}{7! \cdot 3! \cdot 4! \cdot 2!}`.
    Аналогично, можно "заморозить" единицы и представить все оставшиеся числа разными.
    Тогда получим другую формулу:
    :math:`\frac{16 \cdot 15 \cdot 14 \cdot 13 \cdot 12 \cdot 11 \cdot 10 \cdot 9 \cdot 8}{3! \cdot 4! \cdot 2!}`.

.. _bit-parser:

Битовый парсер
--------------

На первый взгляд кажется, что тут всё понятно: надо просто сесть и написать.
Но лучше приглядеться к задаче повнимательнее.

Требуется написать программу, которая побитово разбирает файл и производит некоторые манипуляции согласно файл-описанию.
Первое, что стоит попробовать - это написать тестовые примеры.

Ширина и высота изображения в формате ``BMP``
    Данный пример присутствует в постановке задачи.
    Предлагается ввести команды ``print`` и ``skip``.
    Но что, если нам надо выводить не только числа заданного количества бит, но и буквы, строки, сложные объекты?

Ширина и высота видео в формате ``MP4``
    Согласно стандарту `MPEG-4 Part 14 <https://en.wikipedia.org/wiki/MPEG-4_Part_14>`_ ширина и высота видео хранятся в боксе
    ``stsd`` для трэка видео.
    Маршрут его поиска по боксам такой: ``moov -> trak -> mdia -> minf -> stbl -> stsd``.
    Каждый бокс представляет из себя структуру: размер, кодовое слово (trak, stbl и т.д.), данные.
    Например, на рисунке видны боксы: moov, mvhd, trak, tkhd, edts, elst.
    Перед каждым боксом первые 4 байта - его размер вместе с хранимыми данными.
    Но как можно до них добраться?
    Нужно же уметь сравнивнивать прочитанные биты с каким-то значением и делать выбор.
    То есть необходим аналог ``if`` выражения из языка программирования в файл-описании.

    .. image:: mp4.png

Обратим внимание на пункт в задаче ``ограничения на медиа контейнер``.
Именно им и настало время воспользоваться:

1. распечатывать значения можно только в формате ``int`` с разрядностью от 1 до 64 бит;
2. медиа контейнер должен быть статическим, т.е., распечатываемое значение должно всегда находится в одной и той же битовой позиции от начала файла.

Теперь распишем полную последовательность используемых классов/объектов.

===============  ===========================  =============================  =====================================
Аргументы        -c                           description.json               my.bmp
===============  ===========================  =============================  =====================================
Первый уровень   boost::program_options       boost::property_tree           boost::iostreams::memory_mapped_file
Второй уровень   превращается в вызовы        превращается в упорядоченный   обычный указатель на память
                 ``if(vm.count("config"))``   контейнер ``description``      ``void* memory_ptr``
Третий уровень                                анализ структуры файла         битовый итератор ``BitIterator``
===============  ===========================  =============================  =====================================

Третий уровень является самым подробным.
Анализ структуры файла происходит по обычному циклу:

.. code-block:: cpp

    auto it_file = BitIterator(memory_ptr);
    for (auto it : description)
    {
        switch (it.first)
        {
            case PRINT_COMMAD:
                cout << BitIterator::get(it_file, it.second) << endl;
                break;
            case SKIP_COMMAND:
                it_file += it.second;
                break;
            default:
                cerr << "No such command" << endl;
        }
    }

Мы идём по контейнеру ``description``, который является отображением ``bmp-description.json``.
Итератор контейнера содержит элемент json-а: ``first`` - это имя команды (его мы распарсили где-то заранее), ``second`` - это количество битов.
Пока поддерживаются только команды "напечатать" (print) и "пропустить" (skip) заданное количество битов.
Отсюда следует ещё одно ограничение:

3. поддерживаются только команды ``print`` и ``skip``.

Затем заданное число битов попадает либо в функцию ``BitIterator::get`` и печататается на экран (поэтому мы делали ограничение сверху на 64 бита, чтобы
поместилось в ``uint64_t``), либо просто смещается битовый итератор.
Мы не будем вдаваться в подробности битового интератора, так как это его методы и поведение полностью соответствуют концепции
`итераторов <http://en.cppreference.com/w/cpp/concept/Iterator>`_, для которой задача уже решена много раз в STL.

Использование ``boost::property_tree`` можно заменить на другой парсер.
Самое главное, чтобы на выходе у нас получался контейнер с командами и количествами битов.
``boost::iostreams::memory_mapped_file`` необходим нам, чтобы данные были сразу представлены в виде сырого указателя на память, как будто мы
работаем с обычным ``std::vector``, а также для быстрого считывания больших (по несколько гигабайт) файлов.
Этот подход называется `отображением в память <https://en.wikipedia.org/wiki/Memory-mapped_file>`_.

На этом этапе задачу можно считать решённой.
Остаётся развить битовый парсер дальше, чтобы избавиться от трёх выдвинутых нами ограничений.
