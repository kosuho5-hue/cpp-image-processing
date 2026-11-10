
#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <string>
using namespace std;

struct Header {
    char idLength;
    char colorMapType;
    char dataTypeCode;
    short colorMapOrigin;
    short colorMapLength;
    char colorMapDepth;
    short xOrigin;
    short yOrigin;
    short width;
    short height;
    char bitsPerPixel;
    char imageDescriptor;
};

struct Pixel {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
};

class Image {
public:
    Header header;
    vector<Pixel> pixels;

    void read(const string& filename);
    void write(const string& filename) const;
};
Image multiply(const Image& top, const Image& bottom);
Image subtract(const Image& top, const Image& bottom);
Image screen(const Image& top, const Image& bottom);
Image overlay(const Image& top, const Image& bottom);
#endif
