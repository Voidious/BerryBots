/*
  Copyright (C) 2013 - Voidious

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <fcntl.h>
#include <sstream>
#include <libarchive/archive.h>
#include <libarchive/archive_entry.h>
#include <platformstl/filesystem/filesystem_traits.hpp>
#include "filemanager.h"
#include "guizipper.h"

GuiZipper::GuiZipper() {
  
}

// Based on libarchive's public example code.
// https://github.com/libarchive/libarchive/wiki/Examples#wiki-A_Basic_Write_Example
void GuiZipper::packageFiles(const char *outputFile, const char *baseDir,
    char **filenames, int numFiles, bool binary, const char *absMetaFilename,
    const char *metaFilename) throw (ZipperException*) {
  int r;
  struct archive *a = archive_write_new();
  try {
    r = archive_write_add_filter_gzip(a);
    checkForErrors("Error adding filter gzip", a, r);
    r = archive_write_set_format_pax_restricted(a);
    checkForErrors("Error setting format pax restricted", a, r);
    r = archive_write_open_filename(a, outputFile);
    checkForErrors("Error opening file", a, r);
    packageSingleFile(absMetaFilename, metaFilename, a, false);
    for (int x = 0; x < numFiles; x++) {
      char *filename = filenames[x];
      char *filePath = FileManager::getFilePath(baseDir, filename);
      try {
        packageSingleFile(filePath, filename, a, binary);
      } catch (ZipperException *ze) {
        delete filePath;
        throw ze;
      }
    }
    r = archive_write_close(a);
    checkForErrors("Error writing close", a, r);
    r = archive_write_free(a);
    checkForErrors("Error writing free", a, r);
  } catch (ZipperException *e) {
    archive_write_free(a);
    throw e;
  }
}

void GuiZipper::packageSingleFile(const char *absFilename, const char *filename,
    struct archive *a, bool binary) throw (ZipperException*) {
  struct stat st;
  char buff[8192];
  int fd;
  stat(absFilename, &st);
  struct archive_entry *entry = archive_entry_new();
  try {
    archive_entry_set_size(entry, st.st_size);
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_pathname(entry, filename);
    archive_entry_set_perm(entry, 0644);
    int r = archive_write_header(a, entry);
    checkForErrors("Error writing header", a, r);
    int flags = O_RDONLY;
#ifdef __WIN32__
    if (binary) {
      flags |= O_BINARY;
    }
#endif
    fd = open(absFilename, flags);
    ssize_t len = read(fd, buff, sizeof(buff));
    while (len > 0) {
      archive_write_data(a, buff, len);
      len = read(fd, buff, sizeof(buff));
    }
    close(fd);
    archive_entry_free(entry);
  } catch (ZipperException *e) {
    archive_entry_free(entry);
    throw e;
  }
}

// Based on libarchive's public example code.
// https://github.com/libarchive/libarchive/wiki/Examples#wiki-Constructing_Objects_On_Disk
void GuiZipper::unpackFile(const char *zipFile, const char *outputDir)
    throw (ZipperException*) {
  // TODO: use archive_write_disk_open instead (if/when it exists)
  char cwd[4096];
  getcwd(cwd, 4096);
  char *absZipFile = FileManager::getFilePath(cwd, zipFile);
  platformstl::filesystem_traits<char> traits;
  traits.set_current_directory(outputDir);
  
  struct archive *a;
  struct archive *ext;
  struct archive_entry *entry;
  int flags;
  int r;
  
  flags = ARCHIVE_EXTRACT_TIME;
  flags |= ARCHIVE_EXTRACT_PERM;
  flags |= ARCHIVE_EXTRACT_ACL;
  flags |= ARCHIVE_EXTRACT_FFLAGS;
  
  a = archive_read_new();
  try {
    archive_read_support_format_tar(a);
    archive_read_support_filter_gzip(a);
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);
    r = archive_read_open_filename(a, absZipFile, 10240);
    checkForErrors("Error opening archive for reading", a, r);
    for (;;) {
      r = archive_read_next_header(a, &entry);
      if (r == ARCHIVE_EOF) {
        break;
      }
      checkForErrors("Error reading next archive header", a, r);
      r = archive_write_header(ext, entry);
      checkForErrors("Error writing next archive header", a, r);
      copyData(a, ext, outputDir);
      r = archive_write_finish_entry(ext);
      checkForErrors("Error writing archive finish entry", a, r);
    }
    r = archive_read_close(a);
    checkForErrors("Error closing read archive", a, r);
    r = archive_read_free(a);
    checkForErrors("Error freeing read archive", a, r);
    r = archive_write_close(ext);
    checkForErrors("Error closing write archive", a, r);
    r = archive_write_free(ext);
    checkForErrors("Error freeing write archive", a, r);
  } catch (ZipperException *e) {
    archive_read_free(a);
    archive_write_free(a);
    throw e;
  }
  traits.set_current_directory(cwd);
  delete absZipFile;
}

// Based on libarchive's public example code.
// https://github.com/libarchive/libarchive/wiki/Examples#wiki-Constructing_Objects_On_Disk
ssize_t GuiZipper::copyData(struct archive *ar, struct archive *aw,
                            const char *userDirPath) throw (ZipperException*) {
  ssize_t r;
  const void *buff;
  size_t size;
  int64_t offset;
  for (;;) {
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF) {
      return (ARCHIVE_OK);
    }
    if (r != ARCHIVE_OK) {
      return (r);
    }
    r = archive_write_data_block(aw, buff, size, offset);
    checkForErrors("Error writing archive data block", ar, r);
    return r;
  }
}

void GuiZipper::checkForErrors(const char *message, struct archive *a, long r)
    throw (ZipperException*) {
  if (r != ARCHIVE_OK) {
    std::stringstream msgStream;
    msgStream << message << " (" << r << "): " << archive_error_string(a)
              << " (" << archive_errno(a) << ")";
    throw new ZipperException(msgStream.str().c_str());
  }
}
