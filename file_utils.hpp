#pragma once

namespace physics
{
  template<class T>
  bool LoadFile(const char* filename, vector<T>* buf)
  {
    FILE* f = fopen(filename, "rb");
    if (!f)
      return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    buf->resize(size);
    size_t r = fread(buf->data(), 1, size, f);
    return r == size;
  }

  // Note, the result will be 0 terminated
  bool LoadTextFile(const char* filename, vector<char>* buf);
  bool FileExists(const char* filename);
  string CurrentDirectory();
  bool DirectoryExists(const char* name);
}
