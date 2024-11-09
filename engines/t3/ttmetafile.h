//
// Created by wacha on 10/30/2024.
//

#ifndef T3_TTMETAFILE_H
#define T3_TTMETAFILE_H
#include "common/file.h"
#include "t3seekablereadstream.h"
namespace T3 {

    class TTMetaFile : public Common::SeekableReadStream {
        T3SeekableReadStream *_orgStream;
        int64 _offset, _size;
    public:
        enum ClassNameType {
            HASHED,
            UNHASHED
        };
        TTMetaFile(Common::SeekableReadStream *data, ClassNameType classNameType=UNHASHED);

        ~TTMetaFile();

        bool err() const override;

        void clearErr() override;

        uint32 read(void *dst, uint32 len) override;

        bool eos() const override;

        int64 pos() const override;

        int64 size() const override;

        bool seek(int64 offset, int whence = SEEK_SET) override;


    };
} // T3

#endif //T3_TTMETAFILE_H
