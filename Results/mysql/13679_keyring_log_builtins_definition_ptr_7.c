  char *ptr = new char[len + 1]();
  if (ptr != nullptr) {
    memcpy(ptr, fm, len);
    ptr[len] = 0;
  }
  return ptr;
}

DEFINE_METHOD(void, Log_builtins_keyring::free, (void *ptr)) {
  if (ptr != nullptr) {
    char *mem = (char *)ptr;
    delete[] mem;
  }


// Source: keyring_log_builtins_definition.cc
// Lines 307-319
