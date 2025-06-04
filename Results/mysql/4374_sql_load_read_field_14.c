bool READ_INFO::read_field() {
  int chr, found_enclosed_char;
  uchar *to, *new_buffer;

  found_null = false;
  if (found_end_of_line) return true;  // One have to call next_line

  /* Skip until we find 'line_start' */

  if (start_of_line) {  // Skip until line_start
    start_of_line = false;
    if (find_start_of_fields()) return true;
  }
  if ((chr = GET) == my_b_EOF) {
    found_end_of_line = eof = true;
    return true;
  }
  to = buffer;
  if (chr == enclosed_char) {
    found_enclosed_char = enclosed_char;
    *to++ = (uchar)chr;  // If error
  } else {
    found_enclosed_char = INT_MAX;
    PUSH(chr);
  }

  for (;;) {
    bool escaped_mb = false;
    while (to < end_of_buff) {
      chr = GET;
      if (chr == my_b_EOF) goto found_eof;
      if (chr == escape_char) {
        if ((chr = GET) == my_b_EOF) {
          *to++ = (uchar)escape_char;
          goto found_eof;
        }
        /*
          When escape_char == enclosed_char, we treat it like we do for
          handling quotes in SQL parsing -- you can double-up the
          escape_char to include it literally, but it doesn't do escapes
          like \n. This allows: LOAD DATA ... ENCLOSED BY '"' ESCAPED BY '"'
          with data like: "fie""ld1", "field2"
         */
        if (escape_char != enclosed_char || chr == escape_char) {
          uint ml;
          GET_MBCHARLEN(read_charset, chr, ml);
          /*
            For escaped multibyte character, push back the first byte,
            and will handle it below.
            Because multibyte character's second byte is possible to be
            0x5C, per Query_result_export::send_data, both head byte and
            tail byte are escaped for such characters. So mark it if the
            head byte is escaped and will handle it below.
          */
          if (ml == 1)
            *to++ = (uchar)unescape((char)chr);
          else {
            escaped_mb = true;
            PUSH(chr);
          }
          continue;
        }
        PUSH(chr);
        chr = escape_char;
      }
      if (chr == line_term_char && found_enclosed_char == INT_MAX) {
        if (terminator(line_term_ptr,
                       line_term_length)) {  // Maybe unexpected linefeed
          enclosed = false;
          found_end_of_line = true;
          row_start = buffer;
          row_end = to;
          return false;
        }
      }
      if (chr == found_enclosed_char) {
        if ((chr = GET) == found_enclosed_char) {  // Remove dupplicated
          *to++ = (uchar)chr;
          continue;
        }
        // End of enclosed field if followed by field_term or line_term
        if (chr == my_b_EOF ||
            (chr == line_term_char &&
             terminator(line_term_ptr,
                        line_term_length))) {  // Maybe unexpected linefeed
          enclosed = true;
          found_end_of_line = true;
          row_start = buffer + 1;
          row_end = to;
          return false;
        }
        if (chr == field_term_char &&
            terminator(field_term_ptr, field_term_length)) {
          enclosed = true;
          row_start = buffer + 1;
          row_end = to;
          return false;
        }
        /*
          The string didn't terminate yet.
          Store back next character for the loop
        */
        PUSH(chr);
        /* copy the found term character to 'to' */
        chr = found_enclosed_char;
      } else if (chr == field_term_char && found_enclosed_char == INT_MAX) {
        if (terminator(field_term_ptr, field_term_length)) {
          enclosed = false;
          row_start = buffer;
          row_end = to;
          return false;
        }
      }

      uint ml;
      GET_MBCHARLEN(read_charset, chr, ml);
      if (ml == 0) {
        *to = '\0';
        my_error(ER_INVALID_CHARACTER_STRING, MYF(0), read_charset->csname,
                 buffer);
        error = true;
        return true;
      }

      if (ml > 1 && to + ml <= end_of_buff) {
        uchar *p = to;
        *to++ = chr;

        for (uint i = 1; i < ml; i++) {
          chr = GET;
          if (chr == my_b_EOF) {
            /*
             Need to back up the bytes already ready from illformed
             multi-byte char
            */
            to -= i;
            goto found_eof;
          } else if (chr == escape_char && escaped_mb) {
            // Unescape the second byte if it is escaped.
            chr = GET;
            chr = (uchar)unescape((char)chr);
          }
          *to++ = chr;
        }
        if (escaped_mb) escaped_mb = false;
        if (my_ismbchar(read_charset, (const char *)p, (const char *)to))
          continue;
        for (uint i = 0; i < ml; i++) PUSH(*--to);
        chr = GET;
      } else if (ml > 1) {
        // Buffer is too small, exit while loop, and reallocate.
        PUSH(chr);
        break;
      }
      *to++ = (uchar)chr;
    }
    // We come here if buffer is too small. Enlarge it and continue. Fail if we
    // cannot extend buffer anymore.
    const size_t new_buffer_length = check_length(buff_length, IO_SIZE);
    if ((new_buffer_length == buff_length) ||
        !(new_buffer =
              (uchar *)my_realloc(key_memory_READ_INFO, (char *)buffer,
                                  new_buffer_length + 1, MYF(MY_WME)))) {
      error = true;
      return true;
    }
    to = new_buffer + (to - buffer);
    buffer = new_buffer;
    buff_length = new_buffer_length;
    end_of_buff = buffer + buff_length;
  }

found_eof:
  enclosed = false;
  found_end_of_line = eof = true;
  row_start = buffer;
  row_end = to;
  return false;
}


// Source: sql_load.cc
// Lines 1471-1649
