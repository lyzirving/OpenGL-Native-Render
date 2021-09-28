//
// Created by liuyuzhou on 2021/9/28.
//
#ifndef ENGINE_IMAGE_H
#define ENGINE_IMAGE_H

class Image {
public:
    unsigned char* data{nullptr};
    int width, height, channel;

    Image() { data = nullptr; }
    Image(int inWidth, int inHeight, int inChannel) : width(inWidth), height(inHeight), channel(inChannel) {
        data = nullptr;
    }
    ~Image() {
        delete data;
        data = nullptr;
    }
private:
};

#endif //ENGINE_IMAGE_H
