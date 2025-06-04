static int get_quote_count(const char *line) {
  int quote_count = 0;
  const char *quote = line;

  while ((quote = strpbrk(quote, "'`\"")) != nullptr) {
    quote_count++;
    quote++;
  }

  return quote_count;
}


// Source: mysql.cc
// Lines 4481-4491
