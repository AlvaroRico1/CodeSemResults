  char *Write(int value, char *to) const {
    assert(value >= 0 && value < 100);
    return std::copy_n(m_digits[value], 2, to);
  }

 private:
  char m_digits[100][2]{};
};


// Source: integer_digits.h
// Lines 54-61
