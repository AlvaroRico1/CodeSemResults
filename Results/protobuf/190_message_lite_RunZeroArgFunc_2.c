static void RunZeroArgFunc(const void* arg) {
  void (*func)() = reinterpret_cast<void (*)()>(const_cast<void*>(arg));
  func();
}


// Source: message_lite.cc
// Lines 576-579
