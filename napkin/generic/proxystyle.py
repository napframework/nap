from PyQt5.QtWidgets import *


class MyProxyStyle(QProxyStyle):
    def __init__(self, base):
        super(MyProxyStyle, self).__init__(base)

    def standardPalette(self):
        return super(MyProxyStyle, self).standardPalette()

    # noinspection PyMethodOverriding
    def pixelMetric(self, metric: QStyle.PixelMetric, option: QStyleOption,
                    widget: QWidget):
        if metric == QStyle.PM_LayoutLeftMargin or \
            metric == QStyle.PM_LayoutTopMargin or \
            metric == QStyle.PM_LayoutRightMargin or \
            metric == QStyle.PM_LayoutBottomMargin:
            return 2
        elif metric == QStyle.PM_LayoutHorizontalSpacing or \
            metric == QStyle.PM_LayoutVerticalSpacing:
            return 0

        return super(MyProxyStyle, self).pixelMetric(metric, option, widget)
