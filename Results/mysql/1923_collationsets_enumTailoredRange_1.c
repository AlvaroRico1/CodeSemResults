enumTailoredRange(const void *context, UChar32 start, UChar32 end, uint32_t ce32) {
    if(ce32 == Collation::FALLBACK_CE32) {
        return TRUE;  // fallback to base, not tailored
    }
    TailoredSet *ts = (TailoredSet *)context;
    return ts->handleCE32(start, end, ce32);
}


// Source: collationsets.cpp
// Lines 35-41
