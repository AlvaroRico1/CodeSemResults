int UnescapeCEscapeSequences(const char *source, char *dest,
                             std::vector<std::string> *errors) {
  GOOGLE_DCHECK(errors == nullptr) << "Error reporting not implemented.";

  char* d = dest;
  const char* p = source;

  // Small optimization for case where source = dest and there's no escaping
  while ( p == d && *p != '\0' && *p != '\\' )
    p++, d++;

  while (*p != '\0') {
    if (*p != '\\') {
      *d++ = *p++;
    } else {
      switch ( *++p ) {                    // skip past the '\\'
        case '\0':
          LOG_STRING(ERROR, errors) << "String cannot end with \\";
          *d = '\0';
          return d - dest;   // we're done with p
        case 'a':  *d++ = '\a';  break;
        case 'b':  *d++ = '\b';  break;
        case 'f':  *d++ = '\f';  break;
        case 'n':  *d++ = '\n';  break;
        case 'r':  *d++ = '\r';  break;
        case 't':  *d++ = '\t';  break;
        case 'v':  *d++ = '\v';  break;
        case '\\': *d++ = '\\';  break;
        case '?':  *d++ = '\?';  break;    // \?  Who knew?
        case '\'': *d++ = '\'';  break;
        case '"':  *d++ = '\"';  break;
        case '0': case '1': case '2': case '3':  // octal digit: 1 to 3 digits
        case '4': case '5': case '6': case '7': {
          char ch = *p - '0';
          if ( IS_OCTAL_DIGIT(p[1]) )
            ch = ch * 8 + *++p - '0';
          if ( IS_OCTAL_DIGIT(p[1]) )      // safe (and easy) to do this twice
            ch = ch * 8 + *++p - '0';      // now points at last digit
          *d++ = ch;
          break;
        }
        case 'x': case 'X': {
          if (!isxdigit(p[1])) {
            if (p[1] == '\0') {
              LOG_STRING(ERROR, errors) << "String cannot end with \\x";
            } else {
              LOG_STRING(ERROR, errors) <<
                "\\x cannot be followed by non-hex digit: \\" << *p << p[1];
            }
            break;
          }
          unsigned int ch = 0;
          const char *hex_start = p;
          while (isxdigit(p[1]))  // arbitrarily many hex digits
            ch = (ch << 4) + hex_digit_to_int(*++p);
          if (ch > 0xFF)
            LOG_STRING(ERROR, errors)
                << "Value of "
                << "\\" << std::string(hex_start, p + 1 - hex_start)
                << " exceeds 8 bits";
          *d++ = ch;
          break;
        }


// Source: strutil.cc
// Lines 314-376
