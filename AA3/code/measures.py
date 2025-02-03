import matplotlib.pyplot as plt
from algorithms import *
from typing import Callable

def measures_search(size: int, search_func: Callable[[list[int], int], list[int]], lbl_main: str, sort_opt = 0):
    checks_arr = []
    arr = generate_array(size)
    checks_arr.append(search_func(arr, -1)[1])
    for el in arr:
        checks_arr.append(search_func(arr, el)[1])
    lbl_x = "Позиция найденного элемента"
    lbl_y = "Количество проверок"
    if sort_opt == 1:
        checks_arr.sort()
        lbl_x = "Порядковый номер"
    bar_plot([i for i in range(-1, size)], checks_arr, lbl_main,
             lbl_x, lbl_y)


def bar_plot(arr_x: list[int], arr_y: list[int], lbl_main: str, lbl_x, lbl_y):
    plt.figure(figsize=(10, 10))
    plt.bar(arr_x, arr_y, label=lbl_main)
    plt.legend()
    plt.xlabel(lbl_x)
    plt.ylabel(lbl_y)
    #plt.savefig( "C:\\Users\\redmi\\Downloads\\"+ lbl_main + ".svg", format='svg')
    plt.savefig( "C:\\Users\\redmi\\Downloads\\"+ lbl_main + ".svg", format='svg', bbox_inches='tight')
    plt.show()

x = 8196
size_arr = x // 8 + (x % 1000 if (x >> 2) % 10 == 0 else ((x >> 2) % 10 * (x % 10) + (x >> 1) % 10))

measures_search(size_arr, binary_search, "Бинарный поиск")
measures_search(size_arr, binary_search, "Бинарный поиск с сортировкой", 1)
measures_search(size_arr, full_search, "Полный перебор")
