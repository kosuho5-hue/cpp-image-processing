
#include "Image.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <stdexcept>
using namespace std;

bool isTGA(const string& name) {
    return name.size() >= 4 &&
           name.substr(name.size() - 4) == ".tga";
}

bool fileExists(const string& name) {
    ifstream file(name, ios::binary);
    return file.good();
}

bool parseNumber(const string& text, int& value) {
    try {
        size_t pos = 0;
        long long number = stoll(text, &pos);
        if (pos != text.size() ||
            number < -2147483647LL - 1 ||
            number > 2147483647LL)
            return false;
        value = static_cast<int>(number);
        return true;
    } catch (...) {
        return false;
    }
}

int clampValue(long long value) {
    return static_cast<int>(max(0LL, min(255LL, value)));
}

void flipImage(Image& image) {
    reverse(image.pixels.begin(), image.pixels.end());
}

void onlyChannel(Image& image, const string& method) {
    for (Pixel& p : image.pixels) {
        unsigned char value = 0;

        if (method == "onlyred") value = p.red;
        if (method == "onlygreen") value = p.green;
        if (method == "onlyblue") value = p.blue;

        p.red = value;
        p.green = value;
        p.blue = value;
    }
}

void changeChannel(Image& image, const string& method,
                   int amount) {
    for (Pixel& p : image.pixels) {
        unsigned char* channel = nullptr;

        if (method.find("red") != string::npos)
            channel = &p.red;
        else if (method.find("green") != string::npos)
            channel = &p.green;
        else
            channel = &p.blue;

        long long value = *channel;

        if (method.substr(0, 3) == "add")
            value += amount;
        else
            value *= amount;

        *channel = static_cast<unsigned char>(
            clampValue(value));
    }
}

void combineImages(Image& image,
                   const Image& green,
                   const Image& blue) {
    for (size_t i = 0; i < image.pixels.size(); i++) {
        image.pixels[i].green = green.pixels[i].green;
        image.pixels[i].blue = blue.pixels[i].blue;
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1 ||
        (argc == 2 && string(argv[1]) == "--help")) {
        cout << "Project 2: Image Processing, Fall 2026\n\n";
        cout << "Usage:\n";
        cout << "\t./project2.out [output] [firstImage] [method] [...]\n";
        return 0;
    }

    if (!isTGA(argv[1])) {
        cout << "Invalid file name." << endl;
        return 1;
    }

    if (argc < 3 || !isTGA(argv[2])) {
        cout << "Invalid file name." << endl;
        return 1;
    }

    if (!fileExists(argv[2])) {
        cout << "File does not exist." << endl;
        return 1;
    }

    if (argc < 4) {
        cout << "Invalid method name." << endl;
        return 1;
    }

    try {
        Image tracking;
        tracking.read(argv[2]);

        int i = 3;

        while (i < argc) {
            string method = argv[i++];

            bool imageMethod =
                method == "multiply" ||
                method == "subtract" ||
                method == "screen" ||
                method == "overlay";

            bool numberMethod =
                method == "addred" ||
                method == "addgreen" ||
                method == "addblue" ||
                method == "scalered" ||
                method == "scalegreen" ||
                method == "scaleblue";

            bool channelMethod =
                method == "onlyred" ||
                method == "onlygreen" ||
                method == "onlyblue";

            if (imageMethod) {
                if (i >= argc) {
                    cout << "Missing argument." << endl;
                    return 1;
                }

                string filename = argv[i++];

                if (!isTGA(filename)) {
                    cout << "Invalid argument, invalid file name."
                         << endl;
                    return 1;
                }

                if (!fileExists(filename)) {
                    cout << "Invalid argument, file does not exist."
                         << endl;
                    return 1;
                }

                Image other;
                other.read(filename);

                if (method == "multiply")
                    tracking = multiply(tracking, other);
                else if (method == "subtract")
                    tracking = subtract(tracking, other);
                else if (method == "screen")
                    tracking = screen(other, tracking);
                else
                    tracking = overlay(tracking, other);
            }
            else if (method == "combine") {
                if (i + 1 >= argc) {
                    cout << "Missing argument." << endl;
                    return 1;
                }

                string greenFile = argv[i++];
                string blueFile = argv[i++];

                for (const string& filename :
                     {greenFile, blueFile}) {
                    if (!isTGA(filename)) {
                        cout << "Invalid argument, invalid file name."
                             << endl;
                        return 1;
                    }
                    if (!fileExists(filename)) {
                        cout << "Invalid argument, file does not exist."
                             << endl;
                        return 1;
                    }
                }

                Image green, blue;
                green.read(greenFile);
                blue.read(blueFile);
                combineImages(tracking, green, blue);
            }
            else if (method == "flip") {
                flipImage(tracking);
            }
            else if (channelMethod) {
                onlyChannel(tracking, method);
            }
            else if (numberMethod) {
                if (i >= argc) {
                    cout << "Missing argument." << endl;
                    return 1;
                }

                int amount;
                if (!parseNumber(argv[i++], amount)) {
                    cout << "Invalid argument, expected number."
                         << endl;
                    return 1;
                }

                changeChannel(tracking, method, amount);
            }
            else {
                cout << "Invalid method name." << endl;
                return 1;
            }
        }

        tracking.write(argv[1]);
        cout << "Image processing completed!" << endl;
        return 0;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
