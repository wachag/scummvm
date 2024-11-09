//
// Created by wacha on 11/2/2024.
//

#ifndef TEXTURE_H
#define TEXTURE_H
#include "common/stream.h"
#include "t3/t3.h"
#include "t3/t3seekablereadstream.h"
namespace T3 {

class Texture {
    T3SeekableReadStream * _stream;
public:
    Texture(Common::SeekableReadStream * stream, T3GameType gameType);
};

} // T3

#endif //TEXTURE_H
