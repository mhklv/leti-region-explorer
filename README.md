# Введение

Это библиотека, позволяющая изобразить заданное произвольной функцией
множество точек плоскости на интерактивном графике. Для взаимодействия
с графической подсистемой ОС используется библиотека SDL2. Выложена по
просьбе преподавателя.


# Использование

В библиотеке определены 2 публичные функции:

```
void init_region_explorer(int (*belongs_region)(double, double), int granularity);

int start_region_explorer();
```

`start_region_explorer()` открывает окно с графиком и должна
вызываться после `init_region_explorer()`. 1-й аргумент функции
`init_region_explorer()` -- это указатель на функцию, которая задаёт
область. Она принимает координаты точки и должна возвращать 1, если
заданная точка лежит в окрестности и 0 -- иначе.

Для корректной работы в той же директории, что и исполняемый файл
должна находится директория res со шрифтом.


## Windows

При компиляции программы нужно слинковать regionexplorer.dll:

```
> gcc main.c -L./ -lregionexplorer -o plot.exe
```


## GNU/Linux

На GNU/Linux нужно также добавить текущую директорию в переменную
LD_LIBRARY_PATH или установить библиотеку в другое стандартное
расположение:

```
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`
$ gcc main.c -L./ -lregionexplorer -o plot
```


# Сборка

TODO...
