#pragma once
#include <QtGui/QColor>
#include <QtWidgets/QApplication>
#include <QtGui/QPalette>

static qreal lerp(const qreal& a, const qreal& b, qreal p) {
    return a + (b - a) * p;
}

static QColor lerpCol(const QColor& a, const QColor& b, qreal p) {
    QColor c;
    c.setRgbF(lerp(a.redF(), b.redF(), p),
              lerp(a.greenF(), b.greenF(), p),
              lerp(a.blueF(), b.blueF(), p));
    return c;
}

static const QColor& softForeground() {
    static QColor c = lerpCol(QApplication::palette().color(QPalette::Normal, QPalette::WindowText),
            QApplication::palette().color(QPalette::Disabled, QPalette::WindowText), 0.5);
    return c;
}
static const QColor& softBackground() {
    static QColor c = QApplication::palette().color(QPalette::Normal, QPalette::Window).darker(102);
    return c;
}
