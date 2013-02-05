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

#include <iostream>
#include <libarchive/archive.h>
#include <libarchive/archive_entry.h>
#include <platformstl/filesystem/filesystem_traits.hpp>
#include "filemanager.h"
#include "guizipper.h"

GuiZipper::GuiZipper() {
  
}

GuiZipper::~GuiZipper() {
  
}

// Taken almost verbatim from libarchive's public example code.
// https://github.com/libarchive/libarchive/wiki/Examples#wiki-A_Basic_Write_Example
void GuiZipper::packageFiles(const char *outputFile, const char *baseDir,
    char **filenames, int numFiles, const char *absMetaFilename,
    const char *metaFilename) {
  struct archive *a = archive_write_new();
  archive_write_add_filter_gzip(a);
  archive_write_set_format_pax_restricted(a);
  archive_write_open_filename(a, outputFile);
  packageSingleFile(absMetaFilename, metaFilename, a);
  for (int x = 0; x < numFiles; x++) {
    char *filename = filenames[x];
    char *absFilename = FileManager::getAbsoluteFilename(baseDir, filename);
    packageSingleFile(absFilename, filename, a);
  }
  archive_write_close(a);
  archive_write_free(a);
}

void GuiZipper::packageSingleFile(const char *absFilename,
                                  const char *filename, struct archive *a) {
  struct stat st;
  char buff[8192];
  int fd;
  stat(absFilename, &st);
  struct archive_entry *entry = archive_entry_new();
  archive_entry_set_size(entry, st.st_size);
  archive_entry_set_filetype(entry, AE_IFREG);
  archive_entry_set_pathname(entry, filename);
  archive_entry_set_perm(entry, 0644);
  archive_write_header(a, entry);
  fd = open(absFilename, O_RDONLY);
  ssize_t len = read(fd, buff, sizeof(buff));
  while (len > 0) {
    archive_write_data(a, buff, len);
    len = read(fd, buff, sizeof(buff));
  }
  close(fd);
  archive_entry_free(entry);
  delete absFilename;
}

// Taken almost verbatim from libarchive's public example code.
// https://github.com/libarchive/libarchive/wiki/Examples#wiki-Constructing_Objects_On_Disk
void GuiZipper::unpackFile(const char *zipFile, const char *outputDir) {
  // TODO: use archive_write_disk_open instead (if/when it exists)
  char cwd[4096];
  getcwd(cwd, 4096);
  char *absZipFile = FileManager::getAbsoluteFilename(cwd, zipFile);
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
  archive_read_support_format_tar(a);
  archive_read_support_filter_gzip(a);
  ext = archive_write_disk_new();
  archive_write_disk_set_options(ext, flags);
  archive_write_disk_set_standard_lookup(ext);
  if ((r = archive_read_open_filename(a, absZipFile, 10240))) {
    std::cout << "Error opening archive for reading: " << absZipFile << std::endl;
    exit(1);
  }
  for (;;) {
    r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF) {
      break;
    }
    if (r != ARCHIVE_OK) {
      fprintf(stderr, "%s\n", archive_error_string(a));
    }
    if (r < ARCHIVE_WARN) {
      std::cout << "Error reading next archive header: " << absZipFile << " ("
                << r << ")" << std::endl;
      exit(1);
    }
    r = archive_write_header(ext, entry);
    if (r != ARCHIVE_OK) {
      fprintf(stderr, "%s\n", archive_error_string(ext));
    } else if (archive_entry_size(entry) > 0) {
      copyData(a, ext, outputDir);
      if (r != ARCHIVE_OK) {
        fprintf(stderr, "%s\n", archive_error_string(ext));
      }
      if (r < ARCHIVE_WARN) {
        std::cout << "Error writing next archive header: " << absZipFile << " ("
                  << r << ")" << std::endl;
        exit(1);
      }
    }
    r = archive_write_finish_entry(ext);
    if (r != ARCHIVE_OK) {
      fprintf(stderr, "%s\n", archive_error_string(ext));
    }
    if (r < ARCHIVE_WARN) {
      std::cout << "Error writing finish entry: " << absZipFile << " ("
                << r << ")" << std::endl;
      exit(1);
    }
  }
  archive_read_close(a);
  archive_read_free(a);
  archive_write_close(ext);
  archive_write_free(ext);
  
  traits.set_current_directory(cwd);
  delete absZipFile;
}

// Taken almost verbatim from libarchive's public example code.
// https://github.com/libarchive/libarchive/wiki/Examples#wiki-Constructing_Objects_On_Disk
ssize_t GuiZipper::copyData(struct archive *ar, struct archive *aw,
                            const char *userDirPath) {
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
    if (r != ARCHIVE_OK) {
      fprintf(stderr, "%s\n", archive_error_string(aw));
      return (r);
    }
  }
}
