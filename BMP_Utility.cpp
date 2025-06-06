#include "BMP_Utility.h"

bool BMP_Utility::OpenBMP_BW(const std::string& filename)
{
    file_stream.clear(); //just in case this is not the first Open
    file_stream.open(filename);

    if (file_stream.is_open() == false)
    {
        std::cerr << "Error: file is not found" << std::endl;
        return false;
    }

    if (ReadHeader() == false)
    {
        return false;
    }

    if (IsSupportedBitDepth(bmp_info_header.biBitCount) == false)
    {
        std::cerr << "Error: only 24 and 32 bit depth is supported" << std::endl;
        return false;
    }

    ReadBitmap();

    return true;
}

void BMP_Utility::OutputToConsole_BW() const
{
    for (LONG i = bmp_info_header.biHeight - 1; i >= 0; i--)
    {
        for (LONG j = 0 * i; j < bmp_info_header.biWidth; j++)
        {
            int current_idx = i * bmp_info_header.biWidth + j;
            if (rgb_arr[current_idx].rgbtRed == 255 && rgb_arr[current_idx].rgbtGreen == 255 && rgb_arr[current_idx].rgbtBlue == 255)
            {
                //white
                std::cout << '.' << ' ';
            }
            else
            {
                //any value rather than white
                std::cout << 'B' << ' ';
            }

        }

        std::cout << '\n';
    }

    std::cout << '\n';
}

bool BMP_Utility::IsSupportedBitDepth(WORD bit_count)
{
    if (bit_count == 24 || bit_count == 32)
    {
        return true;
    }

    return false;
}

void BMP_Utility::DrawLine(POINT point_a, POINT point_b, RGBTRIPLE color)
{
    if (point_a.x > bmp_info_header.biHeight - 1 || point_a.y > bmp_info_header.biWidth - 1)
    {
        std::cerr << "Error: DrawLine: Incorrect point a" << std::endl;
        return;
    }
    if (point_b.x > bmp_info_header.biHeight - 1 || point_b.y > bmp_info_header.biWidth - 1)
    {
        std::cerr << "Error: DrawLine: Incorrect point b" << std::endl;
        return;
    }

    if (rgb_arr.empty())
    {
        std::cerr << "Image is empty, can't draw" << std::endl;
        return;
    }

    auto distance = std::sqrt(pow(point_b.x - point_a.x, 2) + pow(point_b.y - point_a.y, 2) * 1.0);

    for (size_t i = 0; i <= distance + 1; i++)
    {
        auto lerp_time = i / distance;
        if (lerp_time > 1)
        {
            lerp_time = 1;
        }

        POINT pt{ std::round( std::lerp(point_a.x, point_b.x, lerp_time) ), std::round( std::lerp(point_a.y, point_b.y, lerp_time) ) };

        rgb_arr[PixelToArrIdx(pt)] = color;

        if (lerp_time == 1)
        {
            break;
        }
    }
}

bool BMP_Utility::SaveBMP(const std::string& filename)
{
    //fstream just won't work, create ofstream

    std::ofstream out_stream;

    out_stream.open(filename, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

    if (file_stream.is_open() == false)
    {
        std::cerr << "Error: could not create file or open existing one for rewriting" << std::endl;
        return false;
    }

    if (rgb_arr.empty())
    {
        std::cerr << "Error: cannot write empty rgb_arr" << std::endl;
        return false;
    }

    auto padding_amount = CalcPaddingAmount();

    //Set correct values before saving
    bmp_info_header.biSize = sizeof(BITMAPINFOHEADER);
    bmp_info_header.biSizeImage = sizeof(rgb_arr[0]) * rgb_arr.size() + padding_amount * bmp_info_header.biHeight; //it common for it to be zero on import, recalc

    bmp_file_header.bfSize = sizeof(bmp_file_header) + sizeof(bmp_info_header) + bmp_info_header.biSizeImage;
    bmp_file_header.bfOffBits = sizeof(bmp_file_header) + sizeof(bmp_info_header);


    out_stream.write(reinterpret_cast<char*>(&bmp_file_header), sizeof(bmp_file_header));
    out_stream.write(reinterpret_cast<char*>(&bmp_info_header), sizeof(bmp_info_header));

    for (size_t i = 0; i < rgb_arr.size(); i++)
    {
        auto just_null = NULL;

        //BGR order for 24 and 32 bit (what we only support) is from Microsoft docs
        out_stream << rgb_arr[i].rgbtBlue;
        out_stream << rgb_arr[i].rgbtGreen;
        out_stream << rgb_arr[i].rgbtRed;
        if (bmp_info_header.biBitCount == 32)
        {
            out_stream.write(reinterpret_cast<char*>(&just_null), 1);
        }

        //add padding bytes if needed
        if (padding_amount > 0 && ArrIdxToPixel(i).y == bmp_info_header.biWidth - 1)
        {
            out_stream.write(reinterpret_cast<char*>(&just_null), padding_amount);
        }

    }

    out_stream.close();

    return true;
}

bool BMP_Utility::ReadHeader()
{
    //Read BITMAPFILEHEADER + BITMAPINFOHEADER
    //Note that we only support BITMAPINFOHEADER. Extended info headers BITMAPV4HEADER or BITMAPV5HEADER is discarded.
    file_stream.read(reinterpret_cast<char*>(&bmp_file_header), sizeof(bmp_file_header));
    file_stream.read(reinterpret_cast<char*>(&bmp_info_header), sizeof(bmp_info_header));

    if (bmp_file_header.bfType != 0x4d42) //0x4d42 = "BM", simply casting "BM" to WORD won't work 
    {
        std::cerr << "Error: Wrong format: the file must start with \"BM\"";
        return false;
    }

    if (bmp_file_header.bfReserved1 != 0 || bmp_file_header.bfReserved2 != 0)
    {
        std::cerr << "Error: Wrong format: Reserved1 or Reserved2 is non-zero";
        return false;
    }

    if (bmp_info_header.biWidth == 0)
    {
        std::cerr << "Error: picture width can't be zero";
        return false;
    }

    if (bmp_info_header.biHeight == 0)
    {
        std::cerr << "Error: picture height can't be zero";
        return false;
    }

    return true;
}


bool BMP_Utility::ReadBitmap()
{
    //should be bmp_info_header.biSizeImage but it might be 0, calculate instead
    const DWORD BITMAP_SIZE = bmp_file_header.bfSize - bmp_file_header.bfOffBits;
    std::vector<char> bitmap(BITMAP_SIZE);

    if (BITMAP_SIZE == 0)
    {
        std::cerr << "Error: wrong bitmap size" << std::endl;
        return false;
    }

    //Explicitly set the stream position cause the 2nd header may have different size
    file_stream.seekg(bmp_file_header.bfOffBits);
    file_stream.read(bitmap.data(), BITMAP_SIZE);

    //DEBUG OUTPUT //outdated and expects 4-bytes, don't use it
    //int line_end = 1;
    //int byte_end = 1;

    //for (size_t i = 0; i < bitmap.size(); i++)
    //{
    //    std::cout << std::to_string(bitmap[i]);
    //    //std::cout << 'n';
    //    if (byte_end == bmp_info_header.biBitCount / 8)
    //    {
    //        std::cout << " / ";
    //        byte_end = 0;
    //    }
    //    if (line_end == ((bmp_info_header.biBitCount / 8) * bmp_info_header.biWidth) + padding_amount)
    //    {
    //        std::cout << std::endl;
    //        line_end = 0;
    //    }
    //    line_end++;
    //    byte_end++;


    //}
    //std::cout << std::endl;
    //DEBUG OUTPUT END


    FillRGBArr(bitmap);

    return true;
}

bool BMP_Utility::FillRGBArr(const std::vector<char>& bitmap)
{
    const int bytes_per_pixel = bmp_info_header.biBitCount / 8;

    auto padding_amount = CalcPaddingAmount();

    //Used to determine when we skip padding
    int padding_counter = 0;

    //Note that by default data is stored from bottom (left corner) to top
    //Might be vise versa but we don't expect this
    for (size_t i = 0;;)
    {
        if (padding_counter == bmp_info_header.biWidth)
        {
            i += padding_amount;
            padding_counter = 0;
        }

        if (i >= bitmap.size())
        {
            break;
        }

        RGBTRIPLE temp_rgb{};

        if (bmp_info_header.biBitCount == 24)
        {
            //BGR order for 24 bit is from Microsoft docs
            temp_rgb.rgbtBlue = bitmap[i];
            temp_rgb.rgbtGreen = bitmap[i + 1];
            temp_rgb.rgbtRed = bitmap[i + 2];
        }
        else if (bmp_info_header.biBitCount == 32)
        {
            //BGRX order for 32 bit is from Microsoft docs
            temp_rgb.rgbtBlue = bitmap[i];
            temp_rgb.rgbtGreen = bitmap[i + 1];
            temp_rgb.rgbtRed = bitmap[i + 2];
            //dicarded: bitmap[i + 3];
        }
        else
        {
            std::cerr << "Error while reading bitmap: unsupported bit depth" << std::endl;
            return false;
        }

        rgb_arr.push_back(temp_rgb);

        padding_counter++;
        i += bytes_per_pixel;
    }

    return true;
}

size_t BMP_Utility::PixelToArrIdx(POINT point) const
{
    return (bmp_info_header.biHeight - 1 - point.x ) * bmp_info_header.biWidth + point.y;
}

POINT BMP_Utility::ArrIdxToPixel(int idx) const
{
    return { (LONG)(idx / (bmp_info_header.biWidth - 1) - 1), idx % bmp_info_header.biWidth };
}

BMP_Utility::~BMP_Utility()
{
    file_stream.close();
}
