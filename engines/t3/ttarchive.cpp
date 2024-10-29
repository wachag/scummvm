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

#include "common/file.h"
#include "common/substream.h"
#include "common/memstream.h"
#include "common/array.h"
#include "common/str.h"

#include "ttarchive.h"

#include <exception>
#include <common/std/vector.h>
#include <twp/util.h>

#include "t3.h"

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

    Common::String TTArchive::readString(Common::SeekableReadStream *_r) {
        uint32_t length = _r->readUint32LE();
        Common::String r;
        warning("Length is %u", length);


        for (uint32_t i = 0; i < length; i++) {
            uint8_t b=_r->readByte();
            if(b!=0)r += b;
        }
        return r;
    }

    bool TTArchive::open(const Common::Path &filename, bool keepStream) {
        _archiveFileName = filename;

        bool result = true;

        Common::File *file = new Common::File();
        if (!file->open(filename)) {
            result = false;
        } else {
            switch (g_t3->getGameType()) {
                case T3Type_TOMI1:
                    result = parseFileTable(file);
                    break;
            }
        }
        file->close();
        delete file;
        return result;
    }

    struct TTArchiveHeader {
        uint32_t version;

        enum InfoMode {
            INFOMODE_NORMAL,
            INFOMODE_ENCRYPTED
        } infoMode;

        enum FilesMode {
            FILESMODE_NORMAL,
            FILESMODE_UNKNOWN,
            FILESMODE_ZIPPED
        } filesMode;

        uint32_t type3;
        uint32_t totalIndex;
        Common::Array<uint32_t> chunks;
        uint32_t reserved0;
        size_t chunkSize;
        uint32_t reserved1;
        uint32_t reserved2;
        uint32_t reserved3;
        uint32_t reserved4;
        uint8_t reserved5;
        uint32_t reserved6;
        uint32_t infoSize;
        uint32_t infoZippedSize;
        Common::Array<uint8_t> fileInformation;
    };

    bool TTArchive::parseFileTable(Common::File *file) {
        TTArchiveHeader header;

        header.version = file->readUint32LE();
        if ((header.version < 1) || (header.version > 9)) {
            error("Invalid version %u\n", header.version);
            return false;
        }

        header.infoMode = static_cast<TTArchiveHeader::InfoMode>(file->readUint32LE());
        if (header.infoMode > 1) {
            error("Invalid info mode %u\n", header.infoMode);
            return false;
        }

        header.type3 = file->readUint32LE();
        header.filesMode = TTArchiveHeader::FILESMODE_NORMAL;
        if (header.version > 2) {
            header.filesMode = static_cast<TTArchiveHeader::FilesMode>(file->readUint32LE());
        }
        if (header.filesMode > 2) {
            error("Invalid files mode %u\n", header.filesMode);
            return false;
        }
        header.totalIndex = 0;
        if (header.version > 2) {
            header.totalIndex = file->readUint32LE();
            warning("Total number of files %u\n", header.totalIndex);
            if (header.totalIndex) {
                header.chunks.resize(header.totalIndex);

                for (uint32_t i = 0; i < header.totalIndex; i++) {
                    header.chunks[i] = file->readUint32LE();
                }
            }
            header.reserved0 = file->readUint32LE();
            if (header.version > 3) {
                header.reserved1 = file->readUint32LE();
                header.reserved2 = file->readUint32LE();
                if (header.version > 6) {
                    header.reserved3 = file->readUint32LE();
                    header.reserved4 = file->readUint32LE();
                    header.chunkSize = file->readUint32LE() * 1024;
                    warning("Chunk size %u\n", header.chunkSize);
                    if (header.version > 7) {
                        header.reserved5 = file->readByte();
                    }
                    if (header.version > 8) { header.reserved6 = file->readUint32LE(); }
                }
            }
        }
        header.infoSize = file->readUint32LE();
        header.infoZippedSize = 0;
        if ((header.filesMode >= TTArchiveHeader::FILESMODE_ZIPPED) && (header.version > 6)) {
            header.infoZippedSize = file->readUint32LE();
        }

        header.fileInformation.resize(header.infoSize);
        if ((header.version > 6) && (header.filesMode >= TTArchiveHeader::FILESMODE_ZIPPED)) {
            error("TODO: unzip");
            return false;
        } else {
            for (uint32_t i = 0; i < header.infoSize; i++) {
                header.fileInformation[i] = file->readByte();
            }
        }
        if (header.infoMode >= header.INFOMODE_ENCRYPTED) {
            error("TODO: decrypt");
            return false;
        }
        size_t baseOffset = file->pos();
        size_t chunksB = 0;
        if (header.filesMode >= TTArchiveHeader::FILESMODE_ZIPPED) {
            if (header.infoMode >= TTArchiveHeader::INFOMODE_ENCRYPTED) {
                chunksB = 1;
            }
        }
        warning("FS compression: %s", header.totalIndex ? "on" : "off");
        warning("FS encryption: %s", chunksB ? "on" : "off");

        Common::MemoryReadStream infoReader(header.fileInformation.data(), header.infoSize);
        uint32_t folders = infoReader.readUint32LE();
        warning("FS folders: %u", folders);
        for (uint32_t i = 0; i < folders; i++) {
            Common::String name = readString(&infoReader);
            warning("FS name: %s", name.c_str());
        }
        uint32_t files = infoReader.readUint32LE();
        warning("FS files: %u", files);
        for (uint32_t i = 0; i < files; i++) {
            Common::String name = readString(&infoReader);

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
            Common::File *file = new Common::File();
            file->open(_archiveFileName);
            return new Common::SeekableSubReadStream(file, i->_offset, i->_offset + i->_len, DisposeAfterUse::YES);
        } else {
            byte *data = static_cast<byte*>(malloc(sizeof(byte) * i->_len));
            _stream->seek(i->_offset, SEEK_SET);
            _stream->read(data, i->_len);
            return new Common::MemoryReadStream(data, i->_len, DisposeAfterUse::YES);
        }
    }
} // T3
