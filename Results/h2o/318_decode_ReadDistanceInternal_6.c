static BROTLI_INLINE BROTLI_BOOL ReadDistanceInternal(
    int safe, BrotliDecoderState* s, BrotliBitReader* br) {
  int distval;
  BrotliBitReaderState memento;
  HuffmanCode* distance_tree = s->distance_hgroup.htrees[s->dist_htree_index];
  if (!safe) {
    s->distance_code = (int)ReadSymbol(distance_tree, br);
  } else {
    uint32_t code;
    BrotliBitReaderSaveState(br, &memento);
    if (!SafeReadSymbol(distance_tree, br, &code)) {
      return BROTLI_FALSE;
    }
    s->distance_code = (int)code;
  }


// Source: decode.c
// Lines 1449-1463
