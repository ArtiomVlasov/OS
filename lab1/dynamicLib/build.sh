#!/bin/bash

set -e

echo "Компиляция dynamic_lib.c..."
gcc -Wall -fPIC -c dynamic_lib.c

echo "Создание динамической библиотеки libdynamic.so..."
gcc -shared -o libdynamic.so dynamic_lib.o

echo "Копирование библиотеки в /lib..."
sudo cp libdynamic.so /lib

echo "Обновление кэша динамических библиотек..."
sudo ldconfig

echo "Компиляция hello_with_dynam_lib.c..."
gcc -c hello_with_dynam_lib.c

echo "Создание исполняемого файла hello_with_dynam_lib..."
gcc -o main_dynamic hello_with_dynam_lib.o libdynamic.so

echo "Сборка завершена успешно!"
