from PyQt5.QtWidgets import *

import nap
from patch.graphscene import GraphScene
from patch.graphview import GraphView


class PatchEditor(QWidget):
    """ The main widget holding a patch view amongst others """

    def __init__(self, ctx):
        """
        @type ctx: AppContext
        """
        self.ctx = ctx
        super(PatchEditor, self).__init__()
        self.setLayout(QHBoxLayout())
        self.__patchView = GraphView(ctx)
        self.layout().addWidget(self.__patchView)
        # self.__scene = PatchScene()
        # self.__patchView.setScene(self.__scene)

    def setModel(self, patch):
        if isinstance(patch, nap.Component):
            patch = patch.childOftype('nap::Patch')
        self.__patchView.setScene(GraphScene(self.ctx, patch))