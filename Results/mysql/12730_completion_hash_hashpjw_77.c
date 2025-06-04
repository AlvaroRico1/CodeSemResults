static uint hashpjw(const char *arKey, uint nKeyLength) {
  uint h = 0, g, i;

  for (i = 0; i < nKeyLength; i++) {
    h = (h << 4) + arKey[i];
    if ((g = (h & 0xF0000000))) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }
  return h;
}


// Source: completion_hash.cc
// Lines 34-45
