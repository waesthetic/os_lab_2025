#!/bin/bash

echo "Задание 1"
cd "$(dirname "$0")" # переход в директорию скрипта
echo -e "\nФайлы в lab0:"
ls
echo -e "\nТекущая директория:"
pwd

# Задание 2
echo -e "\nЗадание 2"
mkdir -p hello
touch hello/empty
cp "src/hello.c" "hello"
mv hello/hello.c hello/newhello.c
echo -e "\nФайл создан:"
ls -la hello

# Задание 3
echo -e "\nЗадание 3"
echo -e "\nPing ya.ru 5 раз..."
ping ya.ru -c 5 || echo -e "\nвыруби впн дурачок!!!"
echo -e "\nВерсия gcc:"
gcc -v

# Задание 4
echo -e "\nЗадание 4"
echo -e "\nКомпиляция newhello.c..."
echo "Бип-буп..."
if gcc hello/newhello.c -o hello/hello_world; then
    echo "Скомпиллирован hello_world:"
    ls hello
    echo "Запуск:"
    ./hello/hello_world
else
    echo "не работает, иди чини!"
fi

# Финалочка
echo -e "\nФиналочка"
git add hello/empty && git add hello/newhello.c && git add lab0.sh
git status
git commit -m "feat: added empty & newhello.c files"
echo -e "\nпуш будэ!!!"
git push origin master

echo -e "\nспс за внимание!"