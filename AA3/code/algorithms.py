import random

def full_search(array: list[int], elem: int) -> (int, int):
    checks = 0
    for i in range(len(array)):
        checks += 1
        if array[i] == elem:
            return i, checks
    return -1, checks

def binary_search(array: list[int], elem: int) -> (int, int):
    array.sort()
    left_border = 0
    right_border = len(array) - 1
    checks = 0
    while left_border <= right_border:
        checks += 2
        mid = (left_border + right_border) // 2
        if array[mid] == elem:
            return mid, checks
        elif array[mid] >= elem:
            right_border = mid - 1
        else:
            left_border = mid + 1
    return -1, checks

def generate_array(size: int) -> list[int]:
    arr = [i for i in range(size)]
    random.shuffle(arr)
    return arr