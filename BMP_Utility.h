#pragma once

#include <fstream>
#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <windows.h>

class BMP_Utility
{
public:
    //Open BMP, black and white only
    bool OpenBMP_BW(const std::string& filename);
    
    //Doesn't check anything, any color is considered Black
    void OutputToConsole_BW() const;

    bool IsSupportedBitDepth(WORD bit_count);

    //Coordinate space is always top-down, since it is the most expected
    //Starting from 0; Row, column
    //Default color is black
    void DrawLine(POINT point_a, POINT point_b, RGBTRIPLE color = { 0, 0, 0 });

    bool SaveBMP(const std::string& filename);

    LONG GetWidth() const { return bmp_info_header.biWidth; };

    LONG GetHeight() const { return bmp_info_header.biHeight; };

    ~BMP_Utility();

private:
    bool ReadHeader();

    bool ReadBitmap();

    bool FillRGBArr(const std::vector<char>& bitmap);

    size_t PixelToArrIdx(POINT point) const;
    POINT ArrIdxToPixel(int idx) const;


    //Each line size (in bytes) must be divisible by 4
    // i.e 3 bytes per pixel (24bit) x 10 width = 30 bytes per line, will be padded to 32
    int CalcPaddingAmount() const { return ((4 - (bmp_info_header.biWidth * bmp_info_header.biBitCount / 8) % 4) % 4); }

private:
    BITMAPFILEHEADER bmp_file_header; 
    BITMAPINFOHEADER bmp_info_header; 
    std::vector<RGBTRIPLE> rgb_arr;

    std::fstream file_stream;
};

