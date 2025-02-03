from itertools import permutations
from random import random, randint
import csv
import time
import matplotlib.pyplot as plt
import matplotlib
# жадность alpha = [0.1, 0.25, 0.5, 0.75, 0.9]
# испарение ro = [0.1, 0.25, 0.5, 0.75, 0.9]
def comparison(graph: list[list], csv_name: str):
    etalon_len = naive_algorithm(graph)[0]
    with open(csv_name, 'w') as csvfile:
        csvwriter = csv.writer(csvfile)
        greed_arr = [0.1, 0.25, 0.5, 0.75, 0.9]
        evaporation_arr = [0.1, 0.25, 0.5, 0.75, 0.9]
        days = [10, 20, 40, 80, 160, 320]
        for day in days:
            for greed in greed_arr:
                for evaporation in evaporation_arr:
                    exps = 20
                    len_ant = 1000
                    for i in range(exps):
                        len_ant = min(ants_algorithm(graph, day, 10, greed, evaporation)[0], len_ant)
                    diff = len_ant - etalon_len
                    row = [day, greed, evaporation, len_ant, diff]
                    csvwriter.writerow(row)

def measures(graph: list[list[int]]):
    time_ant = 0.0
    time_naive = 0.0
    measures = 20
    for i in range(measures):
        start = time.process_time()
        naive_algorithm(graph)
        time_naive += time.process_time() - start
    for i in range(measures):
        start = time.process_time()
        ants_algorithm(graph, 100, 10, 0.5, 0.5)
        time_ant += time.process_time() - start
    time_ant /= measures
    time_naive /= measures
    return time_ant, time_naive

def plots(labels: list[str], plots: list[list[float]], xlabel: str, ylabel: str):
    if len(plots) != len(labels):
        return
    plt.figure(figsize=(10, 12))
    font = {'weight' : 'bold',
            'size' : 12}
    matplotlib.rc('font', **font)
    for i in range(len(labels)):
        plt.plot(plots[i][0], plots[i][1], label=labels[i])
    '''plt.plot(plots[0][0], plots[0][1], 'ro', label=labels[0])
    plt.plot(plots[1][0], plots[1][1], 'kx', label=labels[1])
    plt.plot(plots[2][0], plots[2][1], 'r', label=labels[2])
    plt.plot(plots[3][0], plots[3][1], 'k', label=labels[3])'''
    #plt.plot(plots[2][0], plots[2][1], 'kx', label=labels[1])

    plt.xlabel(xlabel)
    plt.ylabel(ylabel)

    plt.legend()
    plt.savefig('plot.svg', bbox_inches='tight', format='svg')
    plt.show()


def naive_algorithm(graph: list[list[int]]) -> (int, list[int]):
    """
    Функция для решения задачи коммивояжера(поиск минимального пути в графе) полным перебором
    :param graph: Матрица смежности некого графа значение graph[i][j] соответствует "цене" прохода
    из вершины i в вершину j
    :return: возвращает минимальную "цену" пути, проходящего по всем
    вершинам графа по одному разу(без цикла),
    а также сам этот путь в формате списка вершин в очередности их обхода
    """
    if len(graph) == 0:
        return -1, None
    if len(graph[0]) != len(graph):
        return -1, None
    n = len(graph)
    # Все варианты прохода по графу (по варианту - не замкнутый маршрут)
    options = list(permutations(range(n)))
    best_route = []
    best_summ = -1
    for option in options:
        summ = 0
        for i in range(len(option) - 1):
            # если нет пути из i в j, такой маршрут не подойдет
            if graph[option[i]][option[i + 1]] <= 0:
                summ = -1
                break
            summ += graph[option[i]][option[i + 1]]
        if summ == -1:
            continue
        if best_summ == -1:
            best_summ = summ
            best_route = option
        elif summ < best_summ:
            best_summ = summ
            best_route = option
    return best_summ, list(best_route)

def ant_search(graph: list[list[int]], pheromone_matrix: list[list[float]],
               greed: float, herding: float, start_point: int) -> (int, list[int]):
    n = len(graph)
    visited = {start_point}
    cur_point = start_point
    sum_route = 0
    route = [start_point]
    while len(visited) < n:
        probabilities = []
        sum_not_visited = 0.0
        for i in range(n):
            if i not in visited:
                sum_not_visited += (((1.0 / graph[cur_point][i]) ** greed) * (pheromone_matrix[cur_point][i] ** herding))
        for i in range(n):
            if i in visited:
                probabilities.append(0.0)
            else:
                upper = (((1.0 / graph[cur_point][i]) ** greed) * (pheromone_matrix[cur_point][i] ** herding))
                probabilities.append(upper / sum_not_visited)
        #print(probabilities)
        epsilon = random()
        sum_prob = 0.0
        ind = 0
        for i in range(n):
            sum_prob += probabilities[i]
            if sum_prob >= epsilon and i not in visited:
                ind = i
                break
        sum_route += graph[cur_point][ind]
        cur_point = ind
        visited.add(cur_point)
        route.append(cur_point)
    return sum_route, list(route)



def ants_algorithm(graph: list[list[int]], days: int, pher_min: float,
                   greed: float, evaporation: float) -> (int, list[int]):
    """
    Функция для решения задачи коммивояжера муравьиным алгоритмом
    :param graph: Матрица смежности некого графа значение graph[i][j] соответствует "цене" прохода
    из вершины i в вершину j
    :param days: Количество "дней" для поиска решения
    :param pher_min: минимальный уровень феромона, ниже которого не может опускаться значение на дуге
    :param greed: коэффициент "жадности"
    :param evaporation: коэффициент "испарения"
    :return: возвращает минимальную найденную "цену" пути, проходящего по всем
    вершинам графа по одному разу(без цикла),
    а также сам этот путь в формате списка вершин в очередности их обхода
    """
    if len(graph) == 0:
        return -1, None
    if len(graph[0]) != len(graph):
        return -1, None
    n = len(graph)
    if greed < 0 or greed > 1:
        return -1, None
    if evaporation< 0 or evaporation > 1:
        return -1, None
    herding = 1.0 - greed
    # общее количество феромона, распределяемого на все дуги каждый день
    pheromone = 0
    for row in graph:
        for el in row:
            pheromone += el
    pheromone_matrix = []
    for i in range(n):
        pheromone_matrix.append([pher_min] * n)
        pheromone_matrix[i][i] = 0.0
    while days > 0:
        today_routes = []
        # Каждый i-й муравей находит свой путь из вершины i
        for i in range(n):
            today_routes.append(ant_search(graph, pheromone_matrix, greed, herding, i))
        # Происходит испарение феромона
        for i in range(n):
            for j in range(n):
                if i != j:
                    pheromone_matrix[i][j] = max(pheromone_matrix[i][j] * (1 - evaporation), pher_min)
        # Для каждого найденного пути на каждую дугу наносится количество феромона, обратно пропорциональное длине пути
        for price, route in today_routes:
            for j in range(n - 1):
                pheromone_matrix[route[j]][route[j + 1]] += pheromone / price
        # Повторение распределения феромона для самых успешных путей
        today_routes.sort(key=lambda x: x[0])
        price_min = today_routes[0][0]
        for (price, route) in today_routes:
            if price > price_min:
                break
            for j in range(n - 1):
                pheromone_matrix[route[j]][route[j + 1]] += 1.0 * pheromone / price
        days -= 1
        if days == 0:
            return today_routes[0][0], today_routes[0][1]


graph = []
for i in range(8):
    graph.append([])
    for j in range(8):
        if i == j:
            graph[i].append(0)
        else:
            graph[i].append(randint(1, 20))

for row in graph:
    print(*row, sep=" ")

plotx = []
plot_naive = []
plot_ants = []
for i in range(1, 10):
    plotx.append(i)
    graph = []
    for j in range(i):
        graph.append([])
        for k in range(i):
            if j == k:
                graph[j].append(0)
            else:
                graph[j].append(randint(1, 20))
    m = measures(graph)
    plot_naive.append(m[1])
    plot_ants.append(m[0])
print(*plotx)
print(*plot_naive)
print(*plot_ants)
plots(["Алгоритм полного перебора", "Муравьиный алгоритм"], [[plotx, plot_naive], [plotx, plot_ants]],
       "Размерность матрицы", "Время работы, с.")