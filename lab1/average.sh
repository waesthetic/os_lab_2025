#!/bin/bash
if [ $# -eq 0 ]; then
    echo "Ошибка: не передано ни одного числа"
    exit 1
fi

SUM=0
for N in "$@"; do
    SUM=$((SUM + N))
done

echo "Количество чисел: $#"
echo "Сумма: $SUM"
echo "Среднее арифметическое: $(echo "scale=2; $SUM / $#" | bc)"
