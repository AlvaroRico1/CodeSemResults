U_CALLCONV dtitvinfHashTableValueComparator(UHashTok val1, UHashTok val2) {
    const UnicodeString* pattern1 = (UnicodeString*)val1.pointer;
    const UnicodeString* pattern2 = (UnicodeString*)val2.pointer;
    UBool ret = TRUE;
    int8_t i;
    for ( i = 0; i < DateIntervalInfo::kMaxIntervalPatternIndex && ret == TRUE; ++i ) {
        ret = (pattern1[i] == pattern2[i]);
    }
    return ret;
}


// Source: dtitvinf.cpp
// Lines 728-737
