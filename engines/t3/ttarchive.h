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
  TTArchiveEntry(const Common::Path &name, uint32 offset, uint32 len, TTArchive *parent);
  Common::String getName() const override { return _name.baseName(); }
  Common::String getFileName() const override { return _name.baseName(); }
  Common::Path getPathInArchive() const override { return _name; }
  Common::SeekableReadStream *createReadStream() const override;
  Common::SeekableReadStream *createReadStreamForAltStream(Common::AltStreamType altStreamType) const override { return nullptr; }
  friend class TTArchive;
 };

 class TTArchive : public Common::Archive {
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
  typedef Common::HashMap<Common::Path, TTArchiveEntryPtr, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> TTArchiveMap;
  TTArchiveMap _entries;
  Common::SeekableReadStream *_stream;
 };


} // T3

#endif //TTARCHIVE_H
