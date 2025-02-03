import csv


with open('8_8.txt', 'w') as out:
    with open('8_8.csv') as csvfile:
        reader = csv.reader(csvfile)
        for row in reader:
            s = ''
            for j in range(len(row)):
                if j == 0:
                    s += f'{row[j]} & '
                elif j == len(row) - 1:
                    s += f'{row[j]} \\\\\\hline\n'
                else:
                    s += f'{row[j]} & '
            out.write(s)
