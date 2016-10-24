#ifndef QVECTOR3DI_H
#define QVECTOR3DI_H

#include <QVector3D>

class QVector3DI : public QVector3D
{
public:
    QVector3DI();
    QVector3DI(int xpos, int ypos, int zpos);
    void setX(int x);
    void setY(int y);
    void setZ(int z);
    int x() const;
    int y() const;
    int z() const;

};

#endif // QVECTOR3DI_H
