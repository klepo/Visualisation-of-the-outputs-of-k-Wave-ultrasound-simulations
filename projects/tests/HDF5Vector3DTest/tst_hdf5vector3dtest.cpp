#include <QString>
#include <QtTest>

#include <hdf5helper.h>

class HDF5Vector3DTest : public QObject
{
    Q_OBJECT

public:
    HDF5Vector3DTest();

private Q_SLOTS:
    void testCase1();
};

HDF5Vector3DTest::HDF5Vector3DTest()
{
}

void HDF5Vector3DTest::testCase1()
{
    QVERIFY2(true, "Failure");


    HDF5Helper::HDF5Vector3D vector0;

    vector0[0] = 5;
    vector0[1] = 6;
    vector0.set(1, 2, 3);
    qDebug() << vector0.x();

    vector0.x(50);

    qDebug() << vector0[0] << vector0[1] << vector0[2];

    HDF5Helper::HDF5Vector3D vector1(1, 5, 6);

    hsize_t *vector = vector1.getVectorPtr();

    vector[0] = 15;
    vector[1] = 16;
    vector[2] = 17;

    qDebug() << vector[0] << vector[1] << vector[2];
    qDebug() << vector1[0] << vector1[1] << vector1[2];

    vector1 = vector0;
    qDebug() << vector1[0] << vector1[1] << vector1[2];

}

QTEST_APPLESS_MAIN(HDF5Vector3DTest)

#include "tst_hdf5vector3dtest.moc"
