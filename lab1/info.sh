#!/bin/bash
echo "Текущий путь:"
pwd

echo "Дата и время:"
date "+%d.%m.%Y %H:%M:%S"

echo "Переменная PATH:"
echo "$PATH" | tr ':' '\n'
