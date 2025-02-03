from algorithms import (levenstein_recursive, levenstein_recursive_memoization,
                        levenstein_iterative, damerau_levenstein_iterative)
import sys
from PyQt5 import QtGui, QtWidgets
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtWidgets import QMessageBox
import math as m


class MainWindow(QtWidgets.QMainWindow):

    def __init__(self):
        super().__init__()

        width = 1500
        height = 500

        self.setFixedWidth(width)
        self.setFixedHeight(height)

        self.setWindowTitle("Вычисление расстояние Левенштейна")

        self.setStyleSheet("QLabel {font-size: 10pt}")
        self.setStyleSheet("QPushButton {font-size: 10pt}")
        self.setStyleSheet("QTableWidget {font-size: 10pt}")

        self.info = QtWidgets.QLabel()
        self.info.setText("Введите две строки, между которыми требуется определить расстояние")
        self.info.adjustSize()

        self.options = QtWidgets.QComboBox()
        self.options.addItem("Алгоритм итерационный(Левенштейн)")
        self.options.addItem("Алгоритм рекурсивный(Левенштейн)")
        self.options.addItem("Алгоритм рекурсиный с мемоизацией(Левенштейн)")
        self.options.addItem("Алгоритм итерационный(Дамерау-Левенштейн)")

        self.fir_word_label = QtWidgets.QLabel()
        self.fir_word_label.setText("Введите первую строку:")
        self.fir_word = QtWidgets.QLineEdit()

        self.sec_word_label = QtWidgets.QLabel()
        self.sec_word_label.setText("Введите вторую строку:")
        self.sec_word = QtWidgets.QLineEdit()

        self.btn = QtWidgets.QPushButton()
        self.btn.setText("Рассчитать")
        self.btn.clicked.connect(self.main)

        self.ans_label = QtWidgets.QLabel()
        self.ans_label.setText("Результат:")
        self.ans = QtWidgets.QLineEdit()

        self.table = QtWidgets.QTableWidget(self)
        self.table.setFont(QtGui.QFont("Times", 9))
        self.table.setColumnCount(5)
        # self.points_table.resizeColumnsToContents()
        for i in range(5):
            self.table.insertRow(i)
        self.table.horizontalHeader().setVisible(False)
        self.table.verticalHeader().setVisible(False)

        self.table_opt = QtWidgets.QRadioButton()
        self.table_opt.setText("Вывести таблицу(итер. алгоритм)")

        w = QtWidgets.QWidget()
        main = QtWidgets.QVBoxLayout()
        grid_layout = QtWidgets.QGridLayout()
        horiz = QtWidgets.QHBoxLayout()

        grid_layout.addWidget(self.fir_word_label, 0, 0)
        grid_layout.addWidget(self.fir_word, 0, 1)
        grid_layout.addWidget(self.sec_word_label, 1, 0)
        grid_layout.addWidget(self.sec_word, 1, 1)
        grid_layout.addWidget(self.ans_label, 2, 0)
        grid_layout.addWidget(self.ans, 2, 1)
        main.addWidget(self.options)
        main.addWidget(self.table_opt)
        main.addLayout(grid_layout)
        main.addWidget(self.btn)

        w.setFont(QtGui.QFont("Times", 12))
        horiz.addLayout(main)
        horiz.addWidget(self.table)
        w.setLayout(horiz)
        self.setCentralWidget(w)
        QTimer.singleShot(10, self.center_window)

    def center_window(self):
        qr = self.frameGeometry()
        cp = self.screen().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())

    def main(self):
        if self.options.currentIndex() == 0:
            w1 = self.fir_word.text()
            w2 = self.sec_word.text()
            ans = levenstein_iterative(w1, w2)
            self.ans.setText(str(ans[0]))
            if self.table_opt.isChecked():
                self.table.clear()
                self.table.setColumnCount(len(w2) + 2)
                self.table.setRowCount(len(w1) + 2)
                for i in range(len(w2)):
                    self.table.setItem(0, i + 2, QtWidgets.QTableWidgetItem(w2[i]))
                for i in range(len(w1)):
                    self.table.setItem(i + 2, 0, QtWidgets.QTableWidgetItem(w1[i]))
                for i in range(len(w1) + 1):
                    for j in range(len(w2) + 1):
                        self.table.setItem(i + 1, j + 1, QtWidgets.QTableWidgetItem(str(ans[1][i][j])))
                self.table.resizeColumnsToContents()

        elif self.options.currentIndex() == 1:
            self.ans.setText(str(levenstein_recursive(self.fir_word.text(), self.sec_word.text())))
        elif self.options.currentIndex() == 2:
            self.ans.setText(str(levenstein_recursive_memoization(self.fir_word.text(), self.sec_word.text())))
        elif self.options.currentIndex() == 3:
            w1 = self.fir_word.text()
            w2 = self.sec_word.text()
            ans = damerau_levenstein_iterative(w1, w2)
            self.ans.setText(str(ans[0]))
            if self.table_opt.isChecked():
                self.table.clear()
                self.table.setColumnCount(len(w2) + 2)
                self.table.setRowCount(len(w1) + 2)
                for i in range(len(w2)):
                    self.table.setItem(0, i + 2, QtWidgets.QTableWidgetItem(w2[i]))
                for i in range(len(w1)):
                    self.table.setItem(i + 2, 0, QtWidgets.QTableWidgetItem(w1[i]))
                for i in range(len(w1) + 1):
                    for j in range(len(w2) + 1):
                        self.table.setItem(i + 1, j + 1, QtWidgets.QTableWidgetItem(str(ans[1][i][j])))
                self.table.resizeColumnsToContents()

    # Функция вывода сообщения об ошибке
    def show_message(self, error):
        self.msg.setIcon(QMessageBox.Warning)
        self.msg.setText(error)
        self.msg.setWindowTitle("Info")
        retval = self.msg.exec_()


app = QtWidgets.QApplication(sys.argv)
window = MainWindow()
window.show()
app.exec_()