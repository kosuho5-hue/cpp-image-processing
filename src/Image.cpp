#include <algorithm>
#include "Image.h"
#include <fstream>
#include <stdexcept>
using namespace std;

void Image::read(const string& filename) {
    ifstream file(filename, ios::binary);

    if (!file.is_open()) {
        throw runtime_error("Cannot open file: " + filename);
    }

    file.read(&header.idLength, 1);
    file.read(&header.colorMapType, 1);
    file.read(&header.dataTypeCode, 1);
    file.read(reinterpret_cast<char*>(&header.colorMapOrigin), 2);
    file.read(reinterpret_cast<char*>(&header.colorMapLength), 2);
    file.read(&header.colorMapDepth, 1);
    file.read(reinterpret_cast<char*>(&header.xOrigin), 2);
    file.read(reinterpret_cast<char*>(&header.yOrigin), 2);
    file.read(reinterpret_cast<char*>(&header.width), 2);
    file.read(reinterpret_cast<char*>(&header.height), 2);
    file.read(&header.bitsPerPixel, 1);
    file.read(&header.imageDescriptor, 1);

    int count = header.width * header.height;
    pixels.resize(count);

    for (int i = 0; i < count; i++) {
        file.read(reinterpret_cast<char*>(&pixels[i].blue), 1);
        file.read(reinterpret_cast<char*>(&pixels[i].green), 1);
        file.read(reinterpret_cast<char*>(&pixels[i].red), 1);
    }

    if (!file) {
        throw runtime_error("Invalid TGA data: " + filename);
    }
}

void Image::write(const string& filename) const {
    ofstream file(filename, ios::binary);

    if (!file.is_open()) {
        throw runtime_error("Cannot write file: " + filename);
    }

    file.write(&header.idLength, 1);
    file.write(&header.colorMapType, 1);
    file.write(&header.dataTypeCode, 1);
    file.write(reinterpret_cast<const char*>(&header.colorMapOrigin), 2);
    file.write(reinterpret_cast<const char*>(&header.colorMapLength), 2);
    file.write(&header.colorMapDepth, 1);
    file.write(reinterpret_cast<const char*>(&header.xOrigin), 2);
    file.write(reinterpret_cast<const char*>(&header.yOrigin), 2);
    file.write(reinterpret_cast<const char*>(&header.width), 2);
    file.write(reinterpret_cast<const char*>(&header.height), 2);
    file.write(&header.bitsPerPixel, 1);
    file.write(&header.imageDescriptor, 1);

    for (const Pixel& pixel : pixels) {
        file.write(reinterpret_cast<const char*>(&pixel.blue), 1);
        file.write(reinterpret_cast<const char*>(&pixel.green), 1);
        file.write(reinterpret_cast<const char*>(&pixel.red), 1);
    }

    if (!file) {
        throw runtime_error("Failed to write TGA: " + filename);
    }
}

Image multiply(const Image& top, const Image& bottom) {
    Image result = top;

    for (size_t i = 0; i < result.pixels.size(); i++) {
        const Pixel& a = top.pixels[i];
        const Pixel& b = bottom.pixels[i];

        result.pixels[i].red =
            (a.red * b.red + 127) / 255;

        result.pixels[i].green =
            (a.green * b.green + 127) / 255;

        result.pixels[i].blue =
            (a.blue * b.blue + 127) / 255;
    }

    return result;
}

Image subtract(const Image& top, const Image& bottom) {
    Image result = top;

    for (size_t i = 0; i < result.pixels.size(); i++) {
        const Pixel& a = top.pixels[i];
        const Pixel& b = bottom.pixels[i];

        result.pixels[i].red =
            max(0, (int)a.red - (int)b.red);

        result.pixels[i].green =
            max(0, (int)a.green - (int)b.green);

        result.pixels[i].blue =
            max(0, (int)a.blue - (int)b.blue);
    }

    return result;
}

Image screen(const Image& top, const Image& bottom) {
    Image result = top;

    for (size_t i = 0; i < result.pixels.size(); i++) {
        const Pixel& a = top.pixels[i];
        const Pixel& b = bottom.pixels[i];

        result.pixels[i].red =
            255 - ((255 - a.red) * (255 - b.red) + 127) / 255;

        result.pixels[i].green =
            255 - ((255 - a.green) * (255 - b.green) + 127) / 255;

        result.pixels[i].blue =
            255 - ((255 - a.blue) * (255 - b.blue) + 127) / 255;
    }

    return result;
}

Image overlay(const Image& top, const Image& bottom) {
    Image result = top;

    for (size_t i = 0; i < result.pixels.size(); i++) {
        const Pixel& a = top.pixels[i];
        const Pixel& b = bottom.pixels[i];

        auto overlayChannel = [](unsigned char topValue,
                                 unsigned char bottomValue) -> unsigned char {
            double a = topValue / 255.0;
            double b = bottomValue / 255.0;

            double value;

            if (b <= 0.5) {
                value = 2.0 * a * b;
            } else {
                value = 1.0 - 2.0 * (1.0 - a) * (1.0 - b);
            }

            return static_cast<unsigned char>(value * 255.0 + 0.5);
        };

        result.pixels[i].red =
            overlayChannel(a.red, b.red);

        result.pixels[i].green =
            overlayChannel(a.green, b.green);

        result.pixels[i].blue =
            overlayChannel(a.blue, b.blue);
    }

    return result;
}
