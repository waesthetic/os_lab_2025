#!/bin/bash
cd "$(dirname "$0")" # переход в директорию скрипта
mkdir -p build

echo "Задание 1"
echo -e "\nСборка swap:"
gcc src/swap/swap.c src/swap/main.c -o build/swap_app && ./build/swap_app || echo "не запускаеца иди чини крути!"

echo -e "\nЗадание 2"
echo -e "\nСборка revert_string:"
gcc src/revert_string/revert_string.c src/revert_string/main.c -o build/revert_app && ./build/revert_app "Hello World" || echo "не запускаеца иди чини крути!"
echo -e "\nЗапуск без аргументов:"
./build/revert_app

echo -e "\nЗадание 3"
echo -e "\nСтатическая библиотека librevert_static.a:"
gcc -c -o build/revert_string.o src/revert_string/revert_string.c -Isrc/revert_string
ar rcs build/librevert_static.a build/revert_string.o
gcc -o build/revert_static src/revert_string/main.c -Isrc/revert_string -Lbuild -lrevert_static
./build/revert_static "staticaly linked"

echo -e "\nДинамическая библиотека librevert.dylib:"
gcc -fPIC -shared -o build/librevert.dylib src/revert_string/revert_string.c -Isrc/revert_string -install_name @rpath/librevert.dylib
gcc -o build/revert_dynamic src/revert_string/main.c -Isrc/revert_string -Lbuild -lrevert -Wl,-rpath,@executable_path
./build/revert_dynamic "dynamicaly linked"

echo -e "\nЗависимости статической программы:"
otool -L build/revert_static
echo "Зависимости динамической программы:"
otool -L build/revert_dynamic

echo -e "\nЗадание 4"
echo -e "\nСборка тестов с той же динамической библиотекой:"
gcc -o build/tests src/tests/tests.c -Isrc/revert_string -I/opt/homebrew/include -L/opt/homebrew/lib -lcunit -Lbuild -lrevert -Wl,-rpath,@executable_path && ./build/tests

# Финалочка
echo -e "\nФиналочка"
git add src/swap/swap.c src/revert_string/revert_string.c lab2.sh
git status
git commit -m "feat: added lab2 script"
echo -e "\nпуш будэ!!!"
git push origin master

echo -e "\nспс за внимание!"
