#include <iostream>
#include <string>

#include "BMP_Utility.h"

int main()
{
    BMP_Utility bmp_utility;

    while (true)
    {
        std::cout << "Enter input BMP file name:" << std::endl;

        std::string filename;

        //filename = "Test1_24bit.bmp";
        //filename = "Test1_24bit_gimp_headerV5.bmp";
        //filename = "Test1_32bit_converter.bmp";
        //filename = "Test1_32bit_gimp_to_paint.bmp";
        //filename = "Test2_24bit_blank.bmp";
        //filename = "Test3_24bit_blank_5x12.bmp";

        std::cin >> filename;
        std::cout << std::endl;

        if (bmp_utility.OpenBMP_BW(filename) == true)
        {
            bmp_utility.OutputToConsole_BW();
            //Draw a cross
            bmp_utility.DrawLine({ 0, 0 }, { bmp_utility.GetHeight() - 1, bmp_utility.GetWidth() - 1});
            bmp_utility.DrawLine({ 0, bmp_utility.GetWidth() - 1 }, { bmp_utility.GetHeight() - 1, 0 });

            bmp_utility.OutputToConsole_BW();

            std::cout << "Enter output BMP file name:" << std::endl;

            std::string output_name;
            std::cin >> output_name;
            std::cout << std::endl;

            bmp_utility.SaveBMP(output_name);
        }
    }

    return 0;
}

