U_CALLCONV ValueComparator(UHashTok val1, UHashTok val2) {
    const UnicodeString* affix_1 = (UnicodeString*)val1.pointer;
    const UnicodeString* affix_2 = (UnicodeString*)val2.pointer;
    return  *affix_1 == *affix_2;
}


// Source: currpinf.cpp
// Lines 42-46
