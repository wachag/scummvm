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
#ifndef TTARCHIVE_H
#define TTARCHIVE_H
#include "common/archive.h"

namespace Common {
    class File;
}

namespace T3 {
    class TTArchive;

    class TTArchiveEntry : public Common::ArchiveMember {
        TTArchive *_parent;
        Common::Path _name;
        uint32 _offset, _len;

    public:
        TTArchiveEntry(const Common::Path &name, uint32 offset, uint32 len,  TTArchive *parent);

        Common::String getName() const override { return _name.baseName(); }
        Common::String getFileName() const override { return _name.baseName(); }
        Common::Path getPathInArchive() const override { return _name; }

        Common::SeekableReadStream *createReadStream() const override;

        Common::SeekableReadStream *createReadStreamForAltStream(Common::AltStreamType altStreamType) const override {
            return nullptr;
        }

        friend class TTArchive;
    };

    class TTArchive : public Common::Archive {
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

        TTArchiveHeader _header;
        size_t _baseOffset;
        class TTArchiveEntryStream: public Common::SeekableReadStream {
            explicit TTArchiveEntryStream(TTArchiveEntry *entry);
            friend class TTArchive;
            Common::Array<uint8> _currentChunk;
            int64 _pos;
            bool _error;
            TTArchive *_parent;
            uint32 _len;
            uint32 _offset;
            uint32 _loadedChunkIndex;

            bool loadCurrentChunk(bool force);
    public:
            bool err() const override;

            void clearErr() override;

            bool eos() const override;

            uint32 read(void *dataPtr, uint32 dataSize) override;

            int64 pos() const override;

            int64 size() const override;

            bool seek(int64 offset, int whence) override;

            ~TTArchiveEntryStream() override;
        };


    public:
        bool open(const Common::Path &filename, bool keepStream = false);

        TTArchive();

        ~TTArchive();

        // Common::Archive implementation
        bool hasFile(const Common::Path &path) const override;

        int listMembers(Common::ArchiveMemberList &list) const override;

        const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;

        Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

    private:
        static Common::String readString(Common::SeekableReadStream *_r);

        bool parseFileTable(Common::File *_f);

        Common::Path _archiveFileName;
        typedef Common::SharedPtr<TTArchiveEntry> TTArchiveEntryPtr;
        typedef Common::HashMap<Common::Path, TTArchiveEntryPtr, Common::Path::IgnoreCase_Hash,
            Common::Path::IgnoreCase_EqualTo> TTArchiveMap;
        TTArchiveMap _entries;
        Common::SeekableReadStream *_stream;
    };
} // T3

#endif //TTARCHIVE_H
