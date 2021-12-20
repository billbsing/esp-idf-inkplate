/*
image_dither.cpp
Inkplate 6 Arduino library
David Zovko, Borna Biro, Denis Vajak, Zvonimir Haramustek @ e-radionica.com
September 24, 2020
https://github.com/e-radionicacom/Inkplate-6-Arduino-library

For support, please reach over forums: forum.e-radionica.com/en
For more info about the product, please check: www.inkplate.io

This code is released under the GNU Lesser General Public License v3.0: https://www.gnu.org/licenses/lgpl-3.0.en.html
Please review the LICENSE file included with this example.
If you have any questions about licensing, please contact techsupport@e-radionica.com
Distributed as-is; no warranty is given.
*/

#include "image.hpp"
#include <algorithm>

uint8_t Image::ditherGetPixelBmp(uint8_t px, int i, int w, bool paletted)
{
    if (paletted)
        px = ditherPalette[px];

    if (getDisplayMode() == DisplayMode::INKPLATE_1BIT)
        px = (uint16_t)px >> 1;

    uint8_t oldPixel = std::min((uint16_t)0xFF, (uint16_t)((uint16_t)ditherBuffer[i] + px));

    uint8_t newPixel = oldPixel & (getDisplayMode() == DisplayMode::INKPLATE_1BIT ? 0b10000000 : 0b11100000);
    uint8_t quantError = oldPixel - newPixel;

    int16_t line_2_offset = e_ink_width + 20;
    ditherBuffer[line_2_offset + i + 0] += (quantError * 5) >> 4;
    if (i != w - 1)
    {
        ditherBuffer[i + 1] += (quantError * 7) >> 4;
        ditherBuffer[line_2_offset + i + 1] += (quantError * 1) >> 4;
    }
    if (i != 0)
        ditherBuffer[line_2_offset + i - 1] += (quantError * 3) >> 4;

    return newPixel >> 5;
}

uint8_t Image::ditherGetPixelJpeg(uint8_t px, int i, int j, int x, int y, int w, int h)
{
    if (blockW == -1)
    {
        blockW = w;
        blockH = h;
    }

    if (getDisplayMode() == DisplayMode::INKPLATE_1BIT)
        px = (uint16_t)px >> 1;

    uint16_t oldPixel = std::min((uint16_t)0xFF, (uint16_t)((uint16_t)px + (uint16_t)jpegDitherBuffer[j + 1][i + 1] +
                                                       (j ? (uint16_t)0 : (uint16_t)ditherBuffer[x + i])));

    uint8_t newPixel = oldPixel & (getDisplayMode() == DisplayMode::INKPLATE_1BIT ? 0b10000000 : 0b11100000);
    uint8_t quantError = oldPixel - newPixel;

    jpegDitherBuffer[j + 1 + 1][i + 0 + 1] += (quantError * 5) >> 4;

    jpegDitherBuffer[j + 0 + 1][i + 1 + 1] += (quantError * 7) >> 4;
    jpegDitherBuffer[j + 1 + 1][i + 1 + 1] += (quantError * 1) >> 4;

    jpegDitherBuffer[j + 1 + 1][i - 1 + 1] += (quantError * 3) >> 4;

    return newPixel >> 5;
}

void Image::ditherSwap(int w)
{
    int16_t line_2_offset = e_ink_width + 20;
    for (int i = 0; i < w; ++i)
    {
        ditherBuffer[i] = ditherBuffer[line_2_offset + i];
        ditherBuffer[line_2_offset + i] = 0;
    }
}

void Image::ditherSwapBlockJpeg(int x)
{
    int16_t line_2_offset = e_ink_width + 20;
    for (int i = 0; i < 18; ++i)
    {
        if (x + i)
            ditherBuffer[line_2_offset + x + i - 1] += jpegDitherBuffer[blockH - 1 + 2][i];
        jpegDitherBuffer[i][0 + 1] = jpegDitherBuffer[i][blockW - 1 + 2];
    }
    for (int j = 0; j < 18; ++j)
        for (int i = 0; i < 18; ++i)
            if (i != 1)
                jpegDitherBuffer[j][i] = 0;

    jpegDitherBuffer[17][1] = 0;
}