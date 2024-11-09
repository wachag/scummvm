//
// Created by wacha on 10/30/2024.
//


#include "ttmetafile.h"

#include <exception>

#include "t3seekablereadstream.h"

namespace T3 {
    TTMetaFile::TTMetaFile(Common::SeekableReadStream *data,
                           TTMetaFile::ClassNameType classNameType): _orgStream(new T3SeekableReadStream(data)),
                                                                     _offset(0) {
        uint32 magic;
        _orgStream->seek(_offset, SEEK_SET);
        magic = _orgStream->readUint32LE();
        warning("Magic: 0x%08x MKTAG: 0x%08x", magic, MKTAG('M','T','R','E'));
        if (magic != MKTAG('M', 'T', 'R', 'E')) {
            warning("Unknown magic: 0x%x", magic);
            _orgStream->seek(_offset, SEEK_SET);
            return;
        }
        uint32 namesLength;
        namesLength = _orgStream->readUint32LE();
        warning("Magic: 0x%08x namesLength: %d", magic, namesLength);
        for (uint32 i = 0; i < namesLength; i++) {
            if (classNameType == TTMetaFile::ClassNameType::HASHED) {
                uint64 typeNameCRC = _orgStream->readUint64LE();
                uint32 versionCRC = _orgStream->readUint32LE();
            } else {
                Common::String name = _orgStream->readTTString();
                uint32 versionCRC = _orgStream->readUint32LE();
            }
        }
        _offset = _orgStream->pos();
        _size = _orgStream->size() - _offset;

    }

    TTMetaFile::~TTMetaFile() {
        delete _orgStream;
    }

    bool TTMetaFile::eos() const {
        return _orgStream->eos();
    }

    bool TTMetaFile::err() const {
        return _orgStream->err();
    }

    int64 TTMetaFile::pos() const {
        return _orgStream->pos() - _offset;
    }

    uint32 TTMetaFile::read(void *dst, uint32 len) {
        return _orgStream->read(dst, len);
    }

    bool TTMetaFile::seek(int64 offset, int whence) {
        if (whence == SEEK_SET) {
            offset += _offset;
        }
        return _orgStream->seek(offset, whence);
    }

    int64 TTMetaFile::size() const {
        return _size;
    }

    void TTMetaFile::clearErr() {
        _orgStream->clearErr();
    }
} // T3
