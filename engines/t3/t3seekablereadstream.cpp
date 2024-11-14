//
// Created by wacha on 11/2/2024.
//
#include "common/textconsole.h"
#include "t3seekablereadstream.h"

namespace T3 {
    T3SeekableReadStream::T3SeekableReadStream(Common::SeekableReadStream *wrapped):SeekableReadStream(),_wrapped(wrapped) {

    }

    T3SeekableReadStream::~T3SeekableReadStream() {
        delete _wrapped;
    }

    bool T3SeekableReadStream::seek(int64 offset, int origin) {
        return _wrapped->seek(offset, origin);
    }

    bool T3SeekableReadStream::eos() const {
        return _wrapped->eos();
    }

    bool T3SeekableReadStream::err() const {
        return _wrapped->err();
    }

    int64 T3SeekableReadStream::pos() const {
        return _wrapped->pos();
    }

    uint32 T3SeekableReadStream::read(void *dst, uint32 len) {
        return _wrapped->read(dst, len);
    }

    int64 T3SeekableReadStream::size() const {
        return _wrapped->size();
    }

    void T3SeekableReadStream::clearErr() {
        _wrapped->clearErr();
    }

	bool T3SeekableReadStream::readTTBoolean() {
		char c= _wrapped->readSByte();
		return c == '1';
	}

    Common::String T3SeekableReadStream::readTTString() {
        uint32 length;
        Common::String ret;
        length=_wrapped->readUint32LE();
        for(int i=0;i<length;i++) {
            int8 byte=_wrapped->readSByte();
            if(byte == 0) {
                error("Invalid character in string");
            }
            ret+=byte;
        }
        return ret;
    }






} // T3