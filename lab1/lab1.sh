#!/bin/bash

cd "$(dirname "$0")" || exit 1   # переход в директорию скрипта

echo "Задание 1"
echo -e "\nЗапуск background.sh в фоновом режиме:"
./src/background.sh &

echo -e "\nmytext.txt:"
echo "123" > mytext.txt
echo "xyz" >> mytext.txt
cat mytext.txt

read -p "тыкни кнопку чтобы удалить компьютер"
clear

echo "Количество символов в файле:"
wc -m mytext.txt

# Задание 2
echo -e "\nЗадание 2\n"
echo "Запись with_cake с pipe, редирект, p2p, арбитраж:"
cat src/cake_rhymes.txt | grep cake > with_cake.txt
echo "Первые три строки результата:"
head -3 with_cake.txt

echo -e "\n'Громкий' rm"
rm nonexistent.txt
echo -e "\n'Тихий' rm:"
rm nonexistent.txt 2>/dev/null

# Задание 3
echo -e "\nЗадание 3"
echo -e "\nПрава на hello.sh сейчас:"
chmod -x src/hello.sh
ls -l src/hello.sh
echo "chmod +x:"
chmod +x src/hello.sh
ls -l src/hello.sh
echo "Запуск:"
./src/hello.sh
echo -e "\nЗапуск info.sh:"
chmod +x info.sh
./info.sh

# Задание 4
echo -e "\nЗадание 4"
echo -e "\nПроверка average.sh на трёх числах:"
chmod +x average.sh
echo -e "\nГенерирация 150 случайных чисел из /dev/random:"
od -An -N300 -d < /dev/random | tr -s ' ' '\n' | grep -v '^$' > numbers.txt
echo -e "\nnumbers.txt в скрипт average.sh:"
./average.sh $(cat numbers.txt)

# Финалочка
wait
echo -e "\nФиналочка"
git add lab1.sh info.sh average.sh
git status
git commit -m "feat: added lab1 scripts"
echo -e "\nпуш будэ!!!"
git push origin master

echo -e "\nспс за внимание!"