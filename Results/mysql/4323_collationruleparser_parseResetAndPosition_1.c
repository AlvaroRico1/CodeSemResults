CollationRuleParser::parseResetAndPosition(UErrorCode &errorCode) {
    if(U_FAILURE(errorCode)) { return UCOL_DEFAULT; }
    int32_t i = skipWhiteSpace(ruleIndex + 1);
    int32_t j;
    UChar c;
    int32_t resetStrength;
    if(rules->compare(i, BEFORE_LENGTH, BEFORE, 0, BEFORE_LENGTH) == 0 &&
            (j = i + BEFORE_LENGTH) < rules->length() &&
            PatternProps::isWhiteSpace(rules->charAt(j)) &&
            ((j = skipWhiteSpace(j + 1)) + 1) < rules->length() &&
            0x31 <= (c = rules->charAt(j)) && c <= 0x33 &&
            rules->charAt(j + 1) == 0x5d) {
        // &[before n] with n=1 or 2 or 3
        resetStrength = UCOL_PRIMARY + (c - 0x31);
        i = skipWhiteSpace(j + 2);
    } else {
        resetStrength = UCOL_IDENTICAL;
    }
    if(i >= rules->length()) {
        setParseError("reset without position", errorCode);
        return UCOL_DEFAULT;
    }
    UnicodeString str;
    if(rules->charAt(i) == 0x5b) {  // '['
        i = parseSpecialPosition(i, str, errorCode);
    } else {
        i = parseTailoringString(i, str, errorCode);
    }
    sink->addReset(resetStrength, str, errorReason, errorCode);
    if(U_FAILURE(errorCode)) { setErrorContext(); }
    ruleIndex = i;
    return resetStrength;
}


// Source: collationruleparser.cpp
// Lines 173-205
