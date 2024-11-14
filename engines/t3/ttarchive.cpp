/* ScummVM - Graphic Adventure Engine
*
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#define FORBIDDEN_SYMBOL_EXCEPTION_exit
#include "common/file.h"
#include "common/substream.h"
#include "common/memstream.h"
#include "common/array.h"
#include "common/str.h"

#include "ttarchive.h"

#include <exception>
#include <common/compression/deflate.h>
#include <common/std/algorithm.h>

#include "t3.h"
#include "t3seekablereadstream.h"

namespace T3 {
    TTArchiveEntry::TTArchiveEntry(const Common::Path &name, uint32 offset, uint32 len, TTArchive *parent) {
        _name = name;
        _offset = offset;
        _len = len;

        _parent = parent;
        _name.toLowercase();
    }

    Common::SeekableReadStream *TTArchiveEntry::createReadStream() const {
        return _parent->createReadStreamForMember(_name);
    }


    TTArchive::TTArchiveEntryStream::TTArchiveEntryStream(TTArchiveEntry *entry) {
        _currentChunk.resize(entry->_parent->_header.chunkSize);
        _pos = 0;
        _offset = entry->_offset;
        _len = entry->_len;
        warning("len is %u",_len);
        _error = false;
        _parent = entry->_parent;
        _loadedChunkIndex = 0;
        loadCurrentChunk(true);
    }

    bool TTArchive::TTArchiveEntryStream::loadCurrentChunk(bool force) {
        int64 chunkIndex = (_offset + _pos) / _parent->_header.chunkSize;
        if (chunkIndex >= _parent->_header.totalIndex) {
            _error = true;
            return false;
        }
        if (!force && (chunkIndex == _loadedChunkIndex)) { return true; }
        warning("loadCurrentChunk: chunkIndex = %ld _loadedChunkIndex = %ld", chunkIndex, _loadedChunkIndex);
        uint64 fileOffset = _parent->_baseOffset;
        for (uint32 i = 0; i < chunkIndex; i++) {
            fileOffset += _parent->_header.chunks[i];
        }
        warning("loadCurrentChunk: fileOffset = %llu", fileOffset);
        _parent->_stream->seek(fileOffset, SEEK_SET);
        Common::Array<uint8> buffer;
        buffer.resize(_parent->_header.chunks[chunkIndex]);
        _parent->_stream->read(buffer.data(), buffer.size());
        _loadedChunkIndex = chunkIndex;
        // TODO: blowfish


        if(Common::inflateZlibHeaderless(_currentChunk.data(), _currentChunk.size(), buffer.data(), buffer.size())) {
            return true;
        } else {
            _error = true;
            return false;
        }
    }

    bool TTArchive::TTArchiveEntryStream::err() const {
    //    warning("TTArchive::TTArchiveEntryStream::err: %s",_error?"true":"false");
        return _error;
    }

    void TTArchive::TTArchiveEntryStream::clearErr() {
        _error = false;
    }

    bool TTArchive::TTArchiveEntryStream::eos() const {
      //  warning("TTArchive::TTArchiveEntryStream::eos: %s",(_pos>=_len)?"true":"false");
        return _pos >= _len;
    }

    uint32 TTArchive::TTArchiveEntryStream::read(void *dataPtr, uint32 dataSize) {
        int64 remaining = dataSize;
        uint8 *pData = static_cast<uint8 *>(dataPtr);
        if (eos() || dataSize==0 || (dataSize > (_len - _pos))) {
            _error = (dataSize>0);
            warning("TTArchive::TTArchiveEntryStream::read: dataSize = %llu", dataSize);
            return 0;
        }
        while (remaining != 0) {
            int64 offsetInChunk = _pos % _parent->_header.chunkSize;
            int64 remainingInChunk = _parent->_header.chunkSize - offsetInChunk;
            int64 transferSize = (remainingInChunk > remaining) ? remaining : remainingInChunk;
            //warning("ofsin %llu remin %llu transf %llu rema %llu pos %llu len %llu", offsetInChunk, remainingInChunk,
            //        transferSize, remaining, _pos, _len);
      //      warning("Pos is %u Len is %u",(unsigned)_pos,(unsigned)_len);
            memcpy(pData, _currentChunk.data()+offsetInChunk, transferSize);
            _pos += transferSize;
            remaining -= transferSize;
            pData += transferSize;
            if(!this->loadCurrentChunk(false)) {
                _error=true;
                return 0;
            }
        }
        return dataSize;
    }

    int64 TTArchive::TTArchiveEntryStream::pos() const {
        return _pos;
    }

    int64 TTArchive::TTArchiveEntryStream::size() const {
        return _len;
    }

    bool TTArchive::TTArchiveEntryStream::seek(int64 offset, int whence) {
        if(whence == SEEK_SET) {
            _pos = offset;
        } else if(whence == SEEK_CUR) {
            _pos += offset;
        } else if(whence == SEEK_END) {
            warning("seek SEEK_END not yet implemented");
            _error = true;
            return false;
        }
        return loadCurrentChunk(false);
    }

    TTArchive::TTArchiveEntryStream::~TTArchiveEntryStream() {
    }

    bool TTArchive::open(const Common::Path &filename, bool keepStream) {
        _archiveFileName = filename;

        bool result = true;

        Common::File *file = new Common::File();
        if (!file->open(filename)) {
            _stream = nullptr;
            result = false;
        } else {
            _stream = file;
            switch (g_t3->getGameType()) {
                case T3Type_TOMI1:
                    result = parseFileTable(file);
                    break;
            }
        }
        return result;
    }


    bool TTArchive::parseFileTable(Common::File *file) {
        _header.version = file->readUint32LE();
        if ((_header.version < 1) || (_header.version > 9)) {
            error("Invalid version %u\n", _header.version);
            return false;
        }

        _header.infoMode = static_cast<TTArchiveHeader::InfoMode>(file->readUint32LE());
        if (_header.infoMode > 1) {
            error("Invalid info mode %u\n", _header.infoMode);
            return false;
        }

        _header.type3 = file->readUint32LE();
        _header.filesMode = TTArchiveHeader::FILESMODE_NORMAL;
        if (_header.version > 2) {
            _header.filesMode = static_cast<TTArchiveHeader::FilesMode>(file->readUint32LE());
        }
        if (_header.filesMode > 2) {
            error("Invalid files mode %u\n", _header.filesMode);
            return false;
        }
        _header.totalIndex = 0;
        _header.chunkSize = 0;
        if (_header.version > 2) {
            _header.totalIndex = file->readUint32LE();
            warning("Total number of files %u\n", _header.totalIndex);
            if (_header.totalIndex) {
                _header.chunks.resize(_header.totalIndex);

                for (uint32_t i = 0; i < _header.totalIndex; i++) {
                    _header.chunks[i] = file->readUint32LE();
                }
            }
            _header.reserved0 = file->readUint32LE();
            if (_header.version > 3) {
                _header.reserved1 = file->readUint32LE();
                _header.reserved2 = file->readUint32LE();
                if (_header.version > 6) {
                    _header.reserved3 = file->readUint32LE();
                    _header.reserved4 = file->readUint32LE();
                    _header.chunkSize = file->readUint32LE() * static_cast<size_t>(1024);
                    if (_header.version > 7) {
                        _header.reserved5 = file->readByte();
                    }
                    if (_header.version > 8) { _header.reserved6 = file->readUint32LE(); }
                }
            }
        }
        _header.infoSize = file->readUint32LE();
        _header.infoZippedSize = 0;
        if ((_header.filesMode >= TTArchiveHeader::FILESMODE_ZIPPED) && (_header.version > 6)) {
            _header.infoZippedSize = file->readUint32LE();
        }

        _header.fileInformation.resize(_header.infoSize);
        if ((_header.version > 6) && (_header.filesMode >= TTArchiveHeader::FILESMODE_ZIPPED)) {
            _header.fileInformation.resize(_header.infoSize);
            Common::Array<uint8_t> zippedFileInformation(_header.infoZippedSize);
            for (uint32_t i = 0; i < _header.infoZippedSize; i++) {
                zippedFileInformation[i] = file->readByte();
            }
            if (!Common::inflateZlibHeaderless(_header.fileInformation.data(), _header.infoSize,
                                               zippedFileInformation.data(), _header.infoZippedSize)) {
                warning("Inflate error");
            }
        } else {
            for (uint32_t i = 0; i < _header.infoSize; i++) {
                _header.fileInformation[i] = file->readByte();
            }
        }
        if (_header.infoMode >= _header.INFOMODE_ENCRYPTED) {
            error("TODO: decrypt");
            return false;
        }
        _baseOffset = file->pos();
        size_t chunksB = 0;
        if (_header.filesMode >= TTArchiveHeader::FILESMODE_ZIPPED) {
            if (_header.infoMode >= TTArchiveHeader::INFOMODE_ENCRYPTED) {
                chunksB = 1;
            }
        }
        warning("FS compression: %s", _header.totalIndex ? "on" : "off");
        warning("FS encryption: %s", chunksB ? "on" : "off");

        Common::MemoryReadStream *infoReaderMem = new Common::MemoryReadStream(
            _header.fileInformation.data(), _header.infoSize);
        T3SeekableReadStream infoReader(infoReaderMem);
        uint32_t folders = infoReader.readUint32LE();
        warning("FS folders: %u", folders);
        for (uint32_t i = 0; i < folders; i++) {
            Common::String name = infoReader.readTTString();
            warning("FS name: %s", name.c_str());
        }
        uint32_t files = infoReader.readUint32LE();
        warning("FS files: %u", files);
        for (uint32_t i = 0; i < files; i++) {
            Common::String name = infoReader.readTTString();

            warning("FS file name: %s", name.c_str());
            uint32_t zero = infoReader.readUint32LE();
            warning("FS zero: %u", zero);
            uint32_t offset = infoReader.readUint32LE();
            warning("FS offset: %u", offset);
            uint32_t len = infoReader.readUint32LE();
            warning("FS len: %u", len);
            Common::Path path(name, Common::Path::kNoSeparator);
            auto *entry = new TTArchiveEntry(path, offset, len, this);
            _entries[path] = TTArchiveEntryPtr(entry);
        }
        return true;
    }

    TTArchive::TTArchive() {
        _stream = nullptr;
    }

    TTArchive::~TTArchive() {
        if (_stream != nullptr) {
            delete _stream;
            _stream = nullptr;
        }
    }

    // Common::Archive implementation
    bool TTArchive::hasFile(const Common::Path &path) const {
        return _entries.contains(path);
    }

    int TTArchive::listMembers(Common::ArchiveMemberList &list) const {
        int count = 0;

        for (TTArchiveMap::const_iterator i = _entries.begin(); i != _entries.end(); ++i) {
            list.push_back(Common::ArchiveMemberList::value_type(i->_value));
            ++count;
        }

        return count;
    }

    const Common::ArchiveMemberPtr TTArchive::getMember(const Common::Path &path) const {
        if (!_entries.contains(path))
            return Common::ArchiveMemberPtr();
        return _entries[path];
    }

    Common::SeekableReadStream *TTArchive::createReadStreamForMember(const Common::Path &path) const {
        if (!hasFile(path))
            return nullptr;

        TTArchiveEntryPtr i = _entries[path];

        if (!_stream) {
            warning("No stream path");
            Common::File *file = new Common::File();
            file->open(_archiveFileName);
            return new Common::SeekableSubReadStream(file, i->_offset, i->_offset + i->_len, DisposeAfterUse::YES);
        } else {
            warning("Stream already  exists: %u %u", i->_offset, i->_len);
            if (_header.totalIndex == 0) {
                byte *data = static_cast<byte *>(malloc(sizeof(byte) * i->_len));

                _stream->seek(i->_offset, SEEK_SET);

                _stream->read(data, i->_len);
                if (_stream->eos() || _stream->err()) {
                    error("Error reading file");
                }
                return new Common::MemoryReadStream(data, i->_len, DisposeAfterUse::YES);
            } else {
                warning("Compressed");
                return new TTArchiveEntryStream(i.get());
            }
        }
    }
} // T3
