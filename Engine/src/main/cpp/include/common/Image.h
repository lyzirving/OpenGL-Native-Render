//
// Created by liuyuzhou on 2021/9/28.
//
#ifndef ENGINE_IMAGE_H
#define ENGINE_IMAGE_H

class Image {
public:
    unsigned char* data{nullptr};
    int width{0}, height{0}, channel{0};

    Image() { data = nullptr; }
    Image(int inWidth, int inHeight, int inChannel) : width(inWidth), height(inHeight), channel(inChannel) {
        data = nullptr;
    }
    ~Image() {
        if (data != nullptr) { free(data); }
        data = nullptr;
    }
private:
};

#endif //ENGINE_IMAGE_H
