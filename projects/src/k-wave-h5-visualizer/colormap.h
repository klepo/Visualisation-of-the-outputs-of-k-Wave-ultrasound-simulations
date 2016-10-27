#ifndef COLORMAP_H
#define COLORMAP_H

#include <QtGlobal>
#include <QtDebug>

class ColorMap
{
public:
    enum Type
    {
        AUTUMN = 0,
        BONE = 1,
        JET = 2,
        WINTER = 3,
        RAINBOW = 4,
        OCEAN = 5,
        SUMMER = 6,
        SPRING = 7,
        COOL = 8,
        HSV = 9,
        PINK = 10,
        HOT = 11
    };

    const static unsigned char data[][768];

    static void applyColorMap(int length, float minValue, float maxValue, float *srcData, unsigned char *dstData, ColorMap::Type type);

private:
    ColorMap();
};

#endif // COLORMAP_H
