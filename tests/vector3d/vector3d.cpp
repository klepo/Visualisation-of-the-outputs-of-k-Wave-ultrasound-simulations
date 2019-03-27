#include <QtTest>

#include <k-wave-h5-helper.h>

class Vector3D : public QObject
{
    Q_OBJECT

public:
    Vector3D();

private Q_SLOTS:
    void testCase1();
    void testCase2();
    void testCase3();
    void testCase4();
    void testCase5();
    void testCase6();
};

Vector3D::Vector3D()
{
}

void Vector3D::testCase1()
{
    H5Helper::Vector3D vector0;

    QVERIFY(vector0.getSize() == 0);
    QVERIFY(vector0.getLength() == 3);
    QVERIFY(vector0.hasZeros() == true);
    QVERIFY(vector0[0] == 0);
    QVERIFY(vector0[1] == 0);
    QVERIFY(vector0[2] == 0);
    QVERIFY(vector0.getVectorPtr()[0] == 0);
    QVERIFY(vector0.getVectorPtr()[1] == 0);
    QVERIFY(vector0.getVectorPtr()[2] == 0);
    QVERIFY(vector0.x() == 0);
    QVERIFY(vector0.y() == 0);
    QVERIFY(vector0.z() == 0);
}

void Vector3D::testCase2()
{
    H5Helper::Vector3D vector0;

    vector0[0] = 1;
    vector0[1] = 2;
    vector0[2] = 3;

    QVERIFY(vector0[0] == 1);
    QVERIFY(vector0[1] == 2);
    QVERIFY(vector0[2] == 3);
    QVERIFY(vector0.x() == 3);
    QVERIFY(vector0.y() == 2);
    QVERIFY(vector0.z() == 1);
    QVERIFY(vector0.getSize() == 1 * 2 * 3);

    vector0.getVectorPtr()[0] = 4;
    vector0.getVectorPtr()[1] = 5;
    vector0.getVectorPtr()[2] = 6;

    QVERIFY(vector0[0] == 4);
    QVERIFY(vector0[1] == 5);
    QVERIFY(vector0[2] == 6);
    QVERIFY(vector0.getSize() == 4 * 5 * 6);
}

void Vector3D::testCase3()
{
    H5Helper::Vector3D vector0(1, 2, 3);

    QVERIFY(vector0[0] == 1);
    QVERIFY(vector0[1] == 2);
    QVERIFY(vector0[2] == 3);

    vector0.set(4, 5, 6);

    QVERIFY(vector0[0] == 4);
    QVERIFY(vector0[1] == 5);
    QVERIFY(vector0[2] == 6);
}

void Vector3D::testCase4()
{
    H5Helper::Vector3D vector0(3);

    QVERIFY(vector0[0] == 3);
    QVERIFY(vector0[1] == 3);
    QVERIFY(vector0[2] == 3);
}

void Vector3D::testCase5()
{
    H5Helper::Vector3D vector0(4);
    vector0.x(3);
    vector0.y(2);
    vector0.z(1);

    QVERIFY(vector0.x() == 3);
    QVERIFY(vector0.y() == 2);
    QVERIFY(vector0.z() == 1);
}

void Vector3D::testCase6()
{
    H5Helper::Vector3D vector0(4);
    H5Helper::Vector3D vector1(2);
    vector0.x(3);
    vector0.y(2);
    vector0.z(1);

    vector1 = vector0;

    QVERIFY(vector1.x() == 3);
    QVERIFY(vector1.y() == 2);
    QVERIFY(vector1.z() == 1);

    H5Helper::Vector3D vector2(vector1);

    QVERIFY(vector2.x() == 3);
    QVERIFY(vector2.y() == 2);
    QVERIFY(vector2.z() == 1);

    H5Helper::Vector3D vector3 = std::move(vector2);

    QVERIFY(vector3.x() == 3);
    QVERIFY(vector3.y() == 2);
    QVERIFY(vector3.z() == 1);
}

QTEST_MAIN(Vector3D)

#include "vector3d.moc"
