#include "abstractwidget.h"

void AbstractWidget::clearLayout(QLayout *layout)
{
    //QLayout *layout = static_cast<QLayout*>(layout);
    int count = layout->count();
    while (--count >= 0) {
        QLayoutItem *child = layout->itemAt(0);
        layout->removeItem(child);
        delete child->widget();
        delete child;
    }
}
