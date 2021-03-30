# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2021 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
from qtpy import QtWidgets
from mantid.simpleapi import AnalysisDataService


class EAAutoPopupTable(QtWidgets.QWidget):

    def __init__(self, table_name=""):
        super(EAAutoPopupTable, self).__init__(None)
        self.table = QtWidgets.QTableWidget(self)
        self.name = table_name

        self.create_table()
        self.populate_table()

        self.setMinimumSize(700, 400)
        self.setup_layout_interface()

    def setup_layout_interface(self):
        self.setObjectName("PopupTable")
        self.vertical_layout = QtWidgets.QVBoxLayout(self)
        self.vertical_layout.setObjectName("verticalLayout")
        self.vertical_layout.addWidget(self.table)

        self.setLayout(self.vertical_layout)

    def create_table(self):
        table = AnalysisDataService.Instance().retrieve(self.name)
        columns = table.getColumnNames()

        self.table.setColumnCount(len(columns))
        self.table.setHorizontalHeaderLabels(columns)

        header = self.table.horizontalHeader()
        for i in range(len(columns)):
            header.setSectionResizeMode(i, QtWidgets.QHeaderView.Stretch)

    def populate_table(self):
        table = AnalysisDataService.Instance().retrieve(self.name)
        table_data = table.toDict()
        table_entries = []

        for i in range(table.rowCount()):
            table_entries.append([])
            for column in table_data:
                table_entries[-1].append(str(table_data[column][i]))

        for entry in table_entries:
            self.add_entry_to_table(entry)

    def add_entry_to_table(self, row_entries):
        assert len(row_entries) == self.table.columnCount()
        row_position = self.table.rowCount()
        self.table.insertRow(row_position)
        for i, entry in enumerate(row_entries):
            table_item = QtWidgets.QTableWidgetItem(entry)
            self.table.setItem(row_position, i, table_item)
