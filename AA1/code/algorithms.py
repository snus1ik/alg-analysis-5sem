import psutil
import os

delete_cost = 1
insert_cost = 1
change_cost = 1
swap_cost = 1

def process_memory():
    process = psutil.Process(os.getpid())
    mem_info = process.memory_info()
    return mem_info.rss

# decorator function
def profile(func):
    def wrapper(*args, **kwargs):

        mem_before = process_memory()
        result = func(*args, **kwargs)
        mem_after = process_memory()
        print("{}:consumed memory: {:,}".format(
            func.__name__,
            mem_before, mem_after, mem_after - mem_before))

        return result
    return wrapper


def levenstein_iterative(fir_word: str, sec_word: str) -> (int, list):
    fir_len = len(fir_word)
    sec_len = len(sec_word)

    matrice = [[0] * (sec_len + 1) for _ in range(fir_len + 1)]

    for i in range(fir_len + 1):
        matrice[i][0] = i
    for i in range(sec_len + 1):
        matrice[0][i] = i

    for i in range(1, fir_len + 1):
        for j in range(1, sec_len + 1):
            # Вставка означает что по таблице идем сверху вниз (добавляем букву к первому слову)
            # При вставке D(s1, s2) = D(s1, s2') + ins_cost; s2 = s2' + "e"
            insert = matrice[i][j - 1] + insert_cost
            # Удаление означает что по таблице идем слева направо (аналог добавления буквы ко второму слову)
            # При удалении D(s1, s2) = D(s1', s2) + del_cost; s1 = s1' + "e"
            delete = matrice[i - 1][j] + delete_cost
            # Изменение буквы означает что идем по диагонали (удаление и вставка одновременно)
            # Однако если добавляемые буквы совпадают, то цена замены равна нулю (замена не производится)
            # иначе равна change_cost
            # При замене D(s1, s2) = D(s1', s2') + {0("e1" == "e2") change_cost иначе}; s1 = s1' + "e1", s2 = s2' + "e2"
            change = matrice[i - 1][j - 1]
            if fir_word[i - 1] != sec_word[j - 1]:
                change += change_cost

            matrice[i][j] = min(insert, delete, change)

    return matrice[fir_len][sec_len], matrice


def levenstein_recursive(fir_word: str, sec_word: str) -> int:
    fir_len = len(fir_word)
    sec_len = len(sec_word)

    def recursion(fir_word: str, fir_len: int, sec_word: str, sec_len: int) -> int:
        if fir_len == 0 or sec_len == 0:
            return max(fir_len, sec_len)

        # При вставке D(s1, s2) = D(s1, s2') + ins_cost; s2 = s2' + "e"
        insert = recursion(fir_word, fir_len, sec_word, sec_len - 1) + insert_cost
        # При удалении D(s1, s2) = D(s1', s2) + del_cost; s1 = s1' + "e"
        delete = recursion(fir_word, fir_len - 1, sec_word, sec_len) + delete_cost
        # При замене D(s1, s2) = D(s1', s2') + {0("e1" == "e2"), change_cost иначе}; s1 = s1' + "e1", s2 = s2' + "e2"
        change = recursion(fir_word, fir_len - 1, sec_word, sec_len - 1)
        if fir_word[fir_len - 1] != sec_word[sec_len - 1]:
            change += change_cost

        return min(insert, delete, change)

    return recursion(fir_word, fir_len, sec_word, sec_len)


def levenstein_recursive_memoization(fir_word: str, sec_word: str) -> int:
    fir_len = len(fir_word)
    sec_len = len(sec_word)
    answers = {}

    def recursion_memoization(fir_word: str, fir_len: int, sec_word: str, sec_len: int) -> int:
        if fir_len == 0 or sec_len == 0:
            return max(fir_len, sec_len)

        key = f"{fir_len} {sec_len}"
        if key in answers:
            return answers[key]

        # При вставке D(s1, s2) = D(s1, s2') + ins_cost; s2 = s2' + "e"
        insert = recursion_memoization(fir_word, fir_len, sec_word, sec_len - 1) + insert_cost
        # При удалении D(s1, s2) = D(s1', s2) + del_cost; s1 = s1' + "e"
        delete = recursion_memoization(fir_word, fir_len - 1, sec_word, sec_len) + delete_cost
        # При замене D(s1, s2) = D(s1', s2') + {0("e1" == "e2"), change_cost иначе}; s1 = s1' + "e1", s2 = s2' + "e2"
        change = recursion_memoization(fir_word, fir_len - 1, sec_word, sec_len - 1)
        if fir_word[fir_len - 1] != sec_word[sec_len - 1]:
            change += change_cost

        result = min(insert, delete, change)
        if key not in answers:
            answers[key] = result
        return result

    return recursion_memoization(fir_word, fir_len, sec_word, sec_len)


def damerau_levenstein_iterative(fir_word: str, sec_word: str) -> (int, list):
    fir_len = len(fir_word)
    sec_len = len(sec_word)

    matrice = [[0] * (sec_len + 1) for _ in range(fir_len + 1)]

    for i in range(fir_len + 1):
        matrice[i][0] = i
    for i in range(sec_len + 1):
        matrice[0][i] = i

    for i in range(1, fir_len + 1):
        for j in range(1, sec_len + 1):
            # Вставка означает что по таблице идем сверху вниз (добавляем букву к первому слову)
            # При вставке D(s1, s2) = D(s1, s2') + ins_cost; s2 = s2' + "e"
            insert = matrice[i][j - 1] + insert_cost
            # Удаление означет что по таблице идем слева направо (аналог добавления буквы ко второму слову)
            # При удалении D(s1, s2) = D(s1', s2) + del_cost; s1 = s1' + "e"
            delete = matrice[i - 1][j] + delete_cost
            # Изменение буквы означает что идем по диагонали (удаление и вставка одновременно)
            # Однако если добавляемые буквы совпадают, то цена замены равна нулю (замена не производится)
            # иначе равна change_cost
            # При замене D(s1, s2) = D(s1', s2') + {0("e1" == "e2") change_cost иначе}; s1 = s1' + "e1", s2 = s2' + "e2"
            change = matrice[i - 1][j - 1]
            if fir_word[i - 1] != sec_word[j - 1]:
                change += change_cost
            # Можно также менять в слове буквы местами, для этого как минимум нужно дойти до индексов i >= 1, j >= 1
            # Далее задаем результат swap-а букв условной бесконечностью - суммой insert + delete + change
            # После этого проверяем сам факт возможности swap-а: если последние две буквы первого слова
            # равны последним двум буквам второго слова в обрабном порядке, то новое расстояние Левенштейна равно
            # расстоянию между исходными словами, у которых отсутствуют последние две буквы + swap_cost
            # При swap-е D(s1, s2) = D(s1', s2') + {swap_cost("e1e2" == "e4e3"), inf иначе};
            # s1 = s1' + "e1e2", s2 = s2' + "e3e4"
            swap = insert + delete + change
            if i >= 2 and j >= 2:
                if fir_word[i - 1] == sec_word[j - 2] and fir_word[i - 2] == sec_word[j - 1]:
                    swap = matrice[i - 2][j - 2] + swap_cost

            matrice[i][j] = min(insert, delete, change, swap)

    return matrice[fir_len][sec_len], matrice
