int UTF8GenericScanFastAscii(const UTF8ScanObj* st,
                    const char * str,
                    int str_length,
                    int* bytes_consumed) {
  *bytes_consumed = 0;
  if (str_length == 0) return kExitOK;

  const uint8* isrc =  reinterpret_cast<const uint8*>(str);
  const uint8* src = isrc;
  const uint8* srclimit = isrc + str_length;
  const uint8* srclimit8 = str_length < 7 ? isrc : srclimit - 7;
  int n;
  int rest_consumed;
  int exit_reason;
  do {
    // Check initial few bytes one at a time until 8-byte aligned
    while ((((uintptr_t)src & 0x07) != 0) &&
           (src < srclimit) && (src[0] < 0x80)) {
      src++;
    }
    if (((uintptr_t)src & 0x07) == 0) {
      while ((src < srclimit8) &&
             (((reinterpret_cast<const uint32*>(src)[0] |
                reinterpret_cast<const uint32*>(src)[1]) & 0x80808080) == 0)) {
        src += 8;
      }
    }
    while ((src < srclimit) && (src[0] < 0x80)) {
      src++;
    }
    // Run state table on the rest
    n = src - isrc;
    exit_reason = UTF8GenericScan(st, str + n, str_length - n, &rest_consumed);
    src += rest_consumed;
  } while ( exit_reason == kExitDoAgain );

  *bytes_consumed = src - isrc;
  return exit_reason;
}


// Source: structurally_valid.cc
// Lines 496-534
