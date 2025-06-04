  constexpr int operator()(T x) const {
    constexpr int mid = (MinDigits + MaxDigits) / 2;
    constexpr T pivot = pow10(mid);
    if (x < pivot)
      return DigitCounter<T, MinDigits, mid>()(x);
    else
      return DigitCounter<T, mid + 1, MaxDigits>()(x);
  }


// Source: integer_digits.h
// Lines 92-99
