#!/bin/bash

set -e

echo "Компиляция static_lib.c..."
gcc -Wall -c static_lib.c -o static_lib.o

echo "Создание статической библиотеки libstatic.a..."
ar rc libstatic.a static_lib.o

echo "Компиляция hello_with_stat_lib.c..."
gcc -Wall -c hello_with_stat_lib.c

echo "Создание исполняемого файла static_hello..."
gcc hello_with_stat_lib.o -o main_static -L. -lstatic

echo "Сборка завершена успешно!"
