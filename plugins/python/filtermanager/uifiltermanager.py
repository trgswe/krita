'''
This script is licensed CC 0 1.0, so that you can learn from it.

------ CC 0 1.0 ---------------

The person who associated a work with this deed has dedicated the work to the public domain by waiving all of his or her rights to the work worldwide under copyright law, including all related and neighboring rights, to the extent allowed by law.

You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission.

https://creativecommons.org/publicdomain/zero/1.0/legalcode
'''
from filtermanager import filtermanagerdialog
from filtermanager.components import (filtercombobox, filtermanagertreemodel)
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QFormLayout, QAbstractItemView, QDialogButtonBox,
                             QVBoxLayout, QFrame, QAbstractScrollArea, QWidget,
                             QTreeView)
import krita


class UIFilterManager(object):

    def __init__(self):
        self.mainDialog = filtermanagerdialog.FilterManagerDialog()
        self.mainLayout = QVBoxLayout(self.mainDialog)
        self.formLayout = QFormLayout()
        self.buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        self.kritaInstance = krita.Krita.instance()
        self._filters = sorted(self.kritaInstance.filters())
        self._documents = self.kritaInstance.documents()
        self.treeModel = filtermanagertreemodel.FilterManagerTreeModel(self)

        self.documentsTreeView = QTreeView()
        self.filterComboBox = filtercombobox.FilterComboBox(self)

        self.buttonBox.accepted.connect(self.confirmButton)
        self.buttonBox.rejected.connect(self.mainDialog.close)

        self.documentsTreeView.setSelectionMode(QAbstractItemView.SingleSelection)
        self.mainDialog.setWindowModality(Qt.NonModal)

    def initialize(self):
        self.documentsTreeView.setModel(self.treeModel)
        self.documentsTreeView.setWindowTitle("Document Tree Model")
        self.documentsTreeView.resizeColumnToContents(0)
        self.documentsTreeView.resizeColumnToContents(1)
        self.documentsTreeView.resizeColumnToContents(2)

        self.formLayout.addRow("Filters", self.filterComboBox)

        self.line = QFrame()
        self.line.setFrameShape(QFrame.HLine)
        self.line.setFrameShadow(QFrame.Sunken)

        self.mainLayout.addWidget(self.documentsTreeView)
        self.mainLayout.addLayout(self.formLayout)
        self.mainLayout.addWidget(self.line)
        self.mainLayout.addWidget(self.buttonBox)

        self.mainDialog.resize(500, 300)
        self.mainDialog.setWindowTitle("Filter Manager")
        self.mainDialog.setSizeGripEnabled(True)
        self.mainDialog.show()
        self.mainDialog.activateWindow()

    def confirmButton(self):
        documentsIndexes = []

        selectionModel = self.documentsTreeView.selectionModel()
        for index in selectionModel.selectedRows():
            node = self.treeModel.data(index, Qt.UserRole + 1)
            documentIndex = self.treeModel.data(index, Qt.UserRole + 2)
            _type = self.treeModel.data(index, Qt.UserRole + 3)

            if _type == 'Document':
                self.applyFilterOverDocument(self.documents[documentIndex])
            else:
                self.applyFilterOverNode(node, self.documents[documentIndex])

            documentsIndexes.append(documentIndex)

        self.refreshDocumentsProjections(set(documentsIndexes))

    def refreshDocumentsProjections(self, indexes):
        for index in indexes:
            document = self.documents[index]
            document.refreshProjection()

    def applyFilterOverNode(self, node, document):
        _filter = self.kritaInstance.filter(self.filterComboBox.currentText())
        _filter.apply(node, 0, 0, document.width(), document.height())

    def applyFilterOverDocument(self, document):
        """ This method applies the selected filter just to topLevelNodes, then
            if topLevelNodes are GroupLayers, that filter will not be applied. """

        for node in document.topLevelNodes():
            self.applyFilterOverNode(node, document)

    @property
    def filters(self):
        return self._filters

    @property
    def documents(self):
        return self._documents
