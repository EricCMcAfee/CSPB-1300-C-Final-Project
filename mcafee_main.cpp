/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    <Eric McAee>

- All project requirements fully met? (YES or NO):
    <Yes>

- If no, please explain what you could not get to work:
    <N/a>

- Did you do any optional enhancements? If so, please explain:
    <No>
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <cstring>

using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */
int get_int(fstream &stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel>(width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset + i] = (unsigned char)(value >> (i * 8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>> &image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header, 0, 1, 'B');                                             // ID field
    set_bytes(bmp_header, 1, 1, 'M');                                             // ID field
    set_bytes(bmp_header, 2, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE + array_bytes); // Size of BMP file
    set_bytes(bmp_header, 6, 2, 0);                                               // Reserved
    set_bytes(bmp_header, 8, 2, 0);                                               // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE);              // Pixel array offset

    // DIB Header
    set_bytes(dib_header, 0, 4, DIB_HEADER_SIZE); // DIB header size
    set_bytes(dib_header, 4, 4, width_pixels);    // Width of bitmap in pixels
    set_bytes(dib_header, 8, 4, height_pixels);   // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);              // Number of color planes
    set_bytes(dib_header, 14, 2, 24);             // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);              // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);    // Size of raw bitmap data (including padding)
    set_bytes(dib_header, 24, 4, 2835);           // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);           // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);              // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);              // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char *)bmp_header, sizeof(bmp_header));
    stream.write((char *)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char *)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//
// helper function to prompt user to enter an output filename
string get_output_filename()
{
    string filename;
    cout << "Please select an output filename for your altered image. Please do not use the input filename as it will be overwritten.";
    cout << endl;
    cin >> filename;
    return filename;
}
// helper function to print menu of filter options and prompt user for a selection
// @return the user selection used to determine image processor
int get_selection()
{
    string selection;
    int selection_to_int;
    cout << "Please select a filter/option from the list below.\n";
    cout << "0: Change file selection\n1: Adds Vignette \n2: Adds Clarendon \n3: Grayscale \n4: Rotates 90 Degrees \n5: Rotates (Multiples of 90 Degrees)\n6: Enlarges image in x and y direction (integer values only)\n7: Converts image to high contrast (black and white only)\n8: Lightens image by a scaling factor (integer values only)\n9: Darkens image by a scaling factor (integer values only)\n10: Converts image to only black, white, red, blue, and green\n";
    cout << "(Enter Q to quit.)\n";
    cin >> selection;
    if (selection == "Q" || selection == "q")
    {
        cout << "Thank you for using the CSPB1330 image manipulator. Have a great day!\n";
        selection_to_int = -1;
        return selection_to_int;
    }
    try
    {
        selection_to_int = stoi(selection);
    }
    catch (invalid_argument user_input)
    {
        cout << selection;
        cout << " is not a valid selection. Please try again\n";
        selection_to_int = -2;
    }
    return selection_to_int;
}

// helper function to prompt user to enter an input filename
string get_input_filename()
{
    string filename;
    cout << "Please select a BMP file to process. Remember to include the full pathway if not local, and the extension BMP.";
    cout << endl;
    cin >> filename;
    return filename;
}

// helper function to determine max between 3 ints
int max_int(int a, int b, int c)
{
    if (a >= b && a >= c)
    {
        return a;
    }
    else if (b >= a && b >= c)
    {
        return b;
    }
    else
    {
        return c;
    }
}
// Process 1
// Adds vignette effect to image (dark corners)
vector<vector<Pixel>> process_1(const vector<vector<Pixel>> &image)
{
    double num_rows = image.size();
    double num_columns = image[0].size();
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    for (int row = 0; row < num_rows; row++)
    {
        for (int column = 0; column < num_columns; column++)
        {

            double red = image[row][column].red;
            double green = image[row][column].green;
            double blue = image[row][column].blue;

            double distance = sqrt(pow(column - (num_columns / 2.0), 2) + pow(row - (num_rows / 2.0), 2));
            double scaling_factor = (num_rows - distance) / num_rows;

            double new_red = red * scaling_factor;
            double new_green = green * scaling_factor;
            double new_blue = blue * scaling_factor;

            new_image[row][column].red = new_red;
            new_image[row][column].green = new_green;
            new_image[row][column].blue = new_blue;
        }
    }
    return new_image;
}

// Process 2
// Adds Clarendon effect to image (darks darker and lights lighter) by a scaling factor)
vector<vector<Pixel>> process_2(const vector<vector<Pixel>> &image, double scaling_factor)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    double average_value;
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    for (int row = 0; row < num_rows; row++)
    {
        for (int column = 0; column < num_columns; column++)
        {

            double red = image[row][column].red;
            double green = image[row][column].green;
            double blue = image[row][column].blue;

            double new_red;
            double new_green;
            double new_blue;

            average_value = (red + green + blue) / 3.0;

            if (average_value >= 170)
            {
                new_red = (255 - (255 - red) * scaling_factor);
                new_green = (255 - (255 - green) * scaling_factor);
                new_blue = (255 - (255 - blue) * scaling_factor);
            }

            else if (average_value < 90)
            {
                new_red = red * scaling_factor;
                new_green = green * scaling_factor;
                new_blue = blue * scaling_factor;
            }

            else
            {
                new_red = red;
                new_green = green;
                new_blue = blue;
            }

            new_image[row][column].red = new_red;
            new_image[row][column].green = new_green;
            new_image[row][column].blue = new_blue;
        }
    }
    return new_image;
}

// Process 3
// Grayscale image
vector<vector<Pixel>> process_3(const vector<vector<Pixel>> &image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    double gray_value;
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));
    for (int row = 0; row < num_rows; row++)
    {
        for (int column = 0; column < num_columns; column++)
        {

            double red = image[row][column].red;
            double green = image[row][column].green;
            double blue = image[row][column].blue;

            gray_value = (red + green + blue) / 3.0;

            new_image[row][column].red = gray_value;
            new_image[row][column].green = gray_value;
            new_image[row][column].blue = gray_value;
        }
    }
    return new_image;
}

// Process 4
// Rotates image by 90 degrees clockwise (not counter-clockwise)
vector<vector<Pixel>> process_4(const vector<vector<Pixel>> &image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> new_image(num_columns, vector<Pixel>(num_rows));
    for (int row = 0; row < num_rows; row++)
    {
        for (int column = 0; column < num_columns; column++)
        {
            double red = image[row][column].red;
            double green = image[row][column].green;
            double blue = image[row][column].blue;

            new_image[column][num_rows - 1 - row].red = red;
            new_image[column][num_rows - 1 - row].green = green;
            new_image[column][num_rows - 1 - row].blue = blue;
        }
    }
    return new_image;
}

// Process 5
// Rotates image by a specified number of multiples of 90 degrees clockwise
vector<vector<Pixel>> process_5(const vector<vector<Pixel>> &image, int number)
{
    vector<vector<Pixel>> new_image;
    new_image = image;
    if (number % 360 == 0)
    {
        new_image = image;
    }
    else if (number % 360 == 90)
    {
        new_image = process_4(image);
    }
    else if (number % 360 == 180)
    {
        new_image = process_4(process_4(image));
    }
    else
    {
        new_image = process_4(process_4(process_4(image)));
    }
    return new_image;
}

// Process 6
// Enlarges image width and height by user entered factor
vector<vector<Pixel>> process_6(const vector<vector<Pixel>> &image, int x_scale, int y_scale)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    int new_width = num_columns * x_scale;
    int new_height = num_rows * y_scale;
    vector<vector<Pixel>> new_image(new_height, vector<Pixel>(new_width));

    for (int i = 0; i < new_height; i++)
    {
        for (int j = 0; j < new_width; j++)
        {
            new_image[i][j].red = image[int(i / y_scale)][int(j / x_scale)].red;
            new_image[i][j].green = image[int(i / y_scale)][int(j / x_scale)].green;
            new_image[i][j].blue = image[int(i / y_scale)][int(j / x_scale)].blue;
        }
    }
    return new_image;
}

// Process 7
// Convert image to high contrast (black and white only)
vector<vector<Pixel>> process_7(const vector<vector<Pixel>> &image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    int gray_value;
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));
    for (int row = 0; row < num_rows; row++)
    {
        for (int column = 0; column < num_columns; column++)
        {
            int red = image[row][column].red;
            int green = image[row][column].green;
            int blue = image[row][column].blue;

            gray_value = (red + green + blue) / 3;

            if (gray_value >= (255 / 2))
            {
                new_image[row][column].red = 255;
                new_image[row][column].green = 255;
                new_image[row][column].blue = 255;
            }
            else
            {
                new_image[row][column].red = 0;
                new_image[row][column].green = 0;
                new_image[row][column].blue = 0;
            }
        }
    }
    return new_image;
}
// Process 8
// Lightens image by a scaling factor
vector<vector<Pixel>> process_8(const vector<vector<Pixel>> &image, double scaling_factor)
{

    int num_rows = image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));
    for (int row = 0; row < num_rows; row++)
    {
        for (int column = 0; column < num_columns; column++)
        {
            int red = image[row][column].red;
            int green = image[row][column].green;
            int blue = image[row][column].blue;

            new_image[row][column].red = 255 - ((255 - red) * scaling_factor);
            new_image[row][column].green = 255 - ((255 - green) * scaling_factor);
            new_image[row][column].blue = 255 - ((255 - blue) * scaling_factor);
        }
    }
    return new_image;
}

// Process 9
// Darkens image by a scaling factor
vector<vector<Pixel>> process_9(const vector<vector<Pixel>> &image, double scaling_factor)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));
    for (int row = 0; row < num_rows; row++)
    {
        for (int column = 0; column < num_columns; column++)
        {
            int red = image[row][column].red;
            int green = image[row][column].green;
            int blue = image[row][column].blue;

            new_image[row][column].red = red * scaling_factor;
            new_image[row][column].green = green * scaling_factor;
            new_image[row][column].blue = blue * scaling_factor;
        }
    }
    return new_image;
}

// Process 10
// Converts image to only black, white, red, blue, and green
vector<vector<Pixel>> process_10(const vector<vector<Pixel>> &image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    int max_rgb;
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));
    for (int row = 0; row < num_rows; row++)
    {
        for (int column = 0; column < num_columns; column++)
        {
            int red = image[row][column].red;
            int green = image[row][column].green;
            int blue = image[row][column].blue;

            max_rgb = max_int(red, green, blue);

            if ((red + green + blue) >= 550)
            {
                new_image[row][column].red = 255;
                new_image[row][column].green = 255;
                new_image[row][column].blue = 255;
            }
            else if ((red + green + blue) <= 150)
            {
                new_image[row][column].red = 150;
                new_image[row][column].green = 150;
                new_image[row][column].blue = 150;
            }
            else if (max_rgb == red)
            {
                new_image[row][column].red = 255;
                new_image[row][column].green = 0;
                new_image[row][column].blue = 0;
            }
            else if (max_rgb == green)
            {
                new_image[row][column].red = 0;
                new_image[row][column].green = 255;
                new_image[row][column].blue = 0;
            }
            else if (max_rgb == blue)
            {
                new_image[row][column].red = 0;
                new_image[row][column].green = 0;
                new_image[row][column].blue = 255;
            }
        }
    }
    return new_image;
}

int main()
{
    bool CONTINUE = true;
    while (CONTINUE)
    {
        cout << "Hello and welcome to the CSPB1300 image manipulator!" << endl;
        string input_filename;
        int selected_filter = 0;
        string output_filename;
        do
        {
            if (selected_filter == 0)
            {
                input_filename = get_input_filename();
            }
            cout << "Filename: ";
            cout << input_filename;
            cout << "\n";
            selected_filter = get_selection();
            if (selected_filter == -1)
            {
                return 0;
            }
        } while (selected_filter < 1 || selected_filter > 10);

        vector<vector<Pixel>> image_vector = read_image(input_filename);
        vector<vector<Pixel>> new_image_vector;
        switch (selected_filter)
        {
        case 1:
            new_image_vector = process_1(image_vector);
            break;

        case 2:
            double scaling_factor;
            do
            {
                cout << "Please enter a scaling factor between 0 and 1.";
                cout << endl;
                cin >> scaling_factor;
                if (cin.fail())
                {
                    cout << "Numeric value not entered. Program quitting.";
                    return 1;
                }
            } while (scaling_factor < 0 || scaling_factor > 1);

            new_image_vector = process_2(image_vector, scaling_factor);
            break;
        case 3:
            new_image_vector = process_3(image_vector);
            break;
        case 4:
            new_image_vector = process_4(image_vector);
            break;
        case 5:
            int num_degrees;
            do
            {
                cout << "Please enter number of degrees you wish to rotate (in multiples of 90 only).";
                cout << endl;
                cin >> num_degrees;
                if (cin.fail())
                {
                    cout << "Numeric value not entered. Program quitting.";
                    return 1;
                }
            } while (num_degrees % 90 != 0);

            new_image_vector = process_5(image_vector, num_degrees);
            break;
        case 6:
            int x_factor;
            int y_factor;

            cout << "Please enter factor by which to increase the width (integer values only).";
            cout << endl;
            cin >> x_factor;
            if (cin.fail())
            {
                cout << "Non-integer values not allowed. Program quitting.";
                return 1;
            }
            cout << "Please enter a factor by which to increase the height (integer values only).";
            cout << endl;
            cin >> y_factor;
            if (cin.fail())
            {
                cout << "Non-integer values not allowed. Program quitting.";
                return 1;
            }
            new_image_vector = process_6(image_vector, x_factor, y_factor);
            break;

        case 7:
            new_image_vector = process_7(image_vector);
            break;
        case 8:
            double scaling_factor_light;
            do
            {
                cout << "Please enter a factor by which to lighten the image (between 0 and 1).";
                cout << endl;
                cin >> scaling_factor_light;
                if (cin.fail())
                {
                    cout << "Numeric value not entered. Program quitting.";
                    return 1;
                }
            } while (scaling_factor_light < 0 || scaling_factor_light > 1);

            new_image_vector = process_8(image_vector, scaling_factor_light);
            break;
        case 9:
            double scaling_factor_dark;
            do
            {
                cout << "Please enter a factor by which to darken the image (between 0 and 1).";
                cout << endl;
                cin >> scaling_factor_dark;
                if (cin.fail())
                {
                    cout << "Numeric value not entered. Program quitting.";
                    return 1;
                }
            } while (scaling_factor_dark < 0 || scaling_factor_dark > 1);

            new_image_vector = process_9(image_vector, scaling_factor_dark);
            break;
        case 10:
            new_image_vector = process_10(image_vector);
            break;
        }

        output_filename = get_output_filename();
        bool success = write_image(output_filename, new_image_vector);
        if (success)
        {
            cout << "Please find your altered image named: ";
            cout << output_filename;
            cout << " in your current directory";
            cout << endl;
        }
        else
        {
            cout << "Something went wrong. Please check that you entered a valid filename.";
        }
    }
    return 0;
}