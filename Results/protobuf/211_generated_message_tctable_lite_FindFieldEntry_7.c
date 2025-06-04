const TcParseTableBase::FieldEntry* TcParser::FindFieldEntry(
    const TcParseTableBase* table, uint32_t field_num) {
  const FieldEntry* const field_entries = table->field_entries_begin();

  uint32_t fstart = 1;
  uint32_t adj_fnum = field_num - fstart;

  if (PROTOBUF_PREDICT_TRUE(adj_fnum < 32)) {
    uint32_t skipmap = table->skipmap32;
    uint32_t skipbit = 1 << adj_fnum;
    if (PROTOBUF_PREDICT_FALSE(skipmap & skipbit)) return nullptr;
    skipmap &= skipbit - 1;
#if (__GNUC__ || __clang__) && __POPCNT__
    // Note: here and below, skipmap typically has very few set bits
    // (31 in the worst case, but usually zero) so a loop isn't that
    // bad, and a compiler-generated popcount is typically only
    // worthwhile if the processor itself has hardware popcount support.
    adj_fnum -= __builtin_popcount(skipmap);
#else
    while (skipmap) {
      --adj_fnum;
      skipmap &= skipmap - 1;
    }
#endif
    auto* entry = field_entries + adj_fnum;
    PROTOBUF_ASSUME(entry != nullptr);
    return entry;
  }


// Source: generated_message_tctable_lite.cc
// Lines 192-219
