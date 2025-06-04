inline char *write_digits(T number, int digits, char *to) {
  assert(digits >= count_digits(number));

  // The string is built from the end, starting with the least significant
  // digits.
  char *pos = to + digits;

  // The digits are written in groups of two in order to reduce the number of
  // the relatively expensive modulo and division by 10 operations. If it has an
  // odd number of digits, write the leftover digit separately.
  if (digits % 2 != 0) {
    *--pos = '0' + number % 10;
    number /= 10;
  }

  while (pos > to) {
    pos -= 2;
    write_two_digits(number % 100, pos);
    number /= 100;
  }

  return to + digits;
}


// Source: integer_digits.h
// Lines 141-163
