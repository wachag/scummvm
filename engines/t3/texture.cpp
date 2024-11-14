//
// Created by wacha on 11/2/2024.
//

#include "t3/texture.h"

#include <exception>
#include <common/file.h>

namespace T3 {
    Texture::Texture(Common::SeekableReadStream *stream, T3GameType gameType):_stream(new T3SeekableReadStream(stream)) {

        switch (gameType) {
            case T3GameType::T3Type_TOMI1: {
                uint32 samplerStateBlockSize;
                uint32 samplerState;
                uint32 nameBlockSize;
                uint32 importNameBlockSize;
                Common::String name;
                Common::String importName;

                samplerStateBlockSize = _stream->readUint32LE();
                samplerState = _stream->readUint32LE();
                nameBlockSize=_stream->readUint32LE();
                warning("name block size is : 0x%x", nameBlockSize);
                name=_stream->readTTString();
                warning("Name is %s",name.c_str());
                importNameBlockSize = _stream->readUint32LE();
                importName = _stream->readTTString();
                warning("Import Name is %s",importName.c_str());
                bool hasTextureData = _stream->readTTBoolean();
                bool isMipMapped = _stream->readTTBoolean();
                bool isWrapU = _stream->readTTBoolean();
                bool isWrapV = _stream->readTTBoolean();
//                bool isFiltered = _stream->readTTBoolean();
  //              bool isEmbedMipMaps = _stream->readTTBoolean();
                uint32 numMipLevels = _stream->readUint32LE();
                uint32 d3dFormat = _stream->readUint32LE();
                uint32 width = _stream->readUint32LE();
                uint32 height = _stream->readUint32LE();
                uint32 otherWidth = _stream->readUint32LE();
                uint32 otherHeight = _stream->readUint32LE();
                uint32 unk0 = _stream->readUint32LE();
                uint32 unk1 = _stream->readUint32LE();
                bool unk2 = _stream->readTTBoolean();
                uint32 unk3 = _stream->readUint32LE();
                uint32 unk4 = _stream->readUint32LE();
                
                warning("numMipLevels is %d",numMipLevels);
                warning("d3dFormat is %d",d3dFormat);
                warning("width is %d",width);
                warning("height is %d",height);
                break;
            }
            default: {
                error("Unknown game type");
            }
        }
    }

} // T3