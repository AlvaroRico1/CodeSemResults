bool Tokenizer::ParseInteger(const std::string& text, uint64_t max_value,
                             uint64_t* output) {
  // Sadly, we can't just use strtoul() since it is only 32-bit and strtoull()
  // is non-standard.  I hate the C standard library.  :(

  //  return strtoull(text.c_str(), NULL, 0);

  const char* ptr = text.c_str();
  int base = 10;
  if (ptr[0] == '0') {
    if (ptr[1] == 'x' || ptr[1] == 'X') {
      // This is hex.
      base = 16;
      ptr += 2;
    } else {
      // This is octal.
      base = 8;
    }
  }

  uint64_t result = 0;
  for (; *ptr != '\0'; ptr++) {
    int digit = DigitValue(*ptr);
    if (digit < 0 || digit >= base) {
      // The token provided by Tokenizer is invalid. i.e., 099 is an invalid
      // token, but Tokenizer still think it's integer.
      return false;
    }
    if (static_cast<uint64_t>(digit) > max_value ||
        result > (max_value - digit) / base) {
      // Overflow.
      return false;
    }
    result = result * base + digit;
  }

  *output = result;
  return true;
}


// Source: tokenizer.cc
// Lines 912-950
