//
// Created by wacha on 11/2/2024.
//

#ifndef T3SEEKABLEREADSTREAM_H
#define T3SEEKABLEREADSTREAM_H
#include "common/stream.h"
#include "common/str.h"

namespace T3 {
    class T3SeekableReadStream : public Common::SeekableReadStream {
        Common::SeekableReadStream *_wrapped;

    public:
        explicit T3SeekableReadStream(Common::SeekableReadStream *wrapped);

        ~T3SeekableReadStream();

        bool err() const override;

        void clearErr() override;

        uint32 read(void *dst, uint32 len) override;

        bool eos() const override;

        int64 pos() const override;

        int64 size() const override;

        bool seek(int64 offset, int whence = SEEK_SET) ;

        Common::String readTTString();

        bool readTTBoolean();
    };
} // T3

#endif //T3SEEKABLEREADSTREAM_H
