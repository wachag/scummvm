//
// Created by wacha on 11/2/2024.
//

#include "t3/texture.h"

#include <exception>
#include <common/file.h>

namespace T3 {
    Texture::Texture(Common::SeekableReadStream *stream, T3GameType gameType):_stream(new T3SeekableReadStream(stream)) {
        uint32 nameBlockSize;
        Common::String name;
        Common::DumpFile dumpFile;
        dumpFile.open("texture.txt");
        if (dumpFile.isOpen()) {
            while(!stream->eos()) {
                uint8 ize;
                dumpFile.writeByte(stream->readByte());
            }
            dumpFile.close();
            std::terminate();
        }

        switch (gameType) {
            case T3GameType::T3Type_TOMI1: {
                nameBlockSize=_stream->readUint32LE();
                warning("name block size is : 0x%x", nameBlockSize);
                name=_stream->readTTString();
                warning("Name is %s",name.c_str());
                break;
            }
            default: {
                error("Unknown game type");
            }
        }
    }

} // T3