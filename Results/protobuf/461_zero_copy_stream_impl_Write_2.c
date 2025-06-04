bool FileOutputStream::CopyingFileOutputStream::Write(const void* buffer,
                                                      int size) {
  GOOGLE_CHECK(!is_closed_);
  int total_written = 0;

  const uint8_t* buffer_base = reinterpret_cast<const uint8_t*>(buffer);

  while (total_written < size) {
    int bytes;
    do {
      bytes = write(file_, buffer_base + total_written, size - total_written);
    } while (bytes < 0 && errno == EINTR);

    if (bytes <= 0) {
      // Write error.

      // FIXME(kenton):  According to the man page, if write() returns zero,
      //   there was no error; write() simply did not write anything.  It's
      //   unclear under what circumstances this might happen, but presumably
      //   errno won't be set in this case.  I am confused as to how such an
      //   event should be handled.  For now I'm treating it as an error, since
      //   retrying seems like it could lead to an infinite loop.  I suspect
      //   this never actually happens anyway.

      if (bytes < 0) {
        errno_ = errno;
      }
      return false;
    }
    total_written += bytes;
  }

  return true;
}


// Source: zero_copy_stream_impl.cc
// Lines 214-247
