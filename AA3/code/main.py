from algorithms import *

size = int(input("Enter the size of the array: "))
array = generate_array(size)
print("Your array: ", array)
el = int(input("Enter element: "))

index_sorted = sorted([[i, array[i]] for i in range(size)], key=lambda x: x[1])

ind, checks = full_search(array, el)
print(f"FULL SEARCH index {"found:" if ind != -1 else "not found,"} "
      f"{"" if ind == -1 else f"{ind}, "}checks: {checks}")
ind, checks = binary_search(array, el)
print(f"BINARY SEARCH index {"found:" if ind != -1 else "not found,"} "
      f"{"" if ind == -1 else f"{index_sorted[ind][0]}, "}checks: {checks}")