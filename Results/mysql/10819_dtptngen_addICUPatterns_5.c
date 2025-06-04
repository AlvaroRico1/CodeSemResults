DateTimePatternGenerator::addICUPatterns(const Locale& locale, UErrorCode& status) {
    if (U_FAILURE(status)) { return; }
    UnicodeString dfPattern;
    UnicodeString conflictingString;
    DateFormat* df;

    // Load with ICU patterns
    for (int32_t i=DateFormat::kFull; i<=DateFormat::kShort; i++) {
        DateFormat::EStyle style = (DateFormat::EStyle)i;
        df = DateFormat::createDateInstance(style, locale);
        SimpleDateFormat* sdf;
        if (df != nullptr && (sdf = dynamic_cast<SimpleDateFormat*>(df)) != nullptr) {
            sdf->toPattern(dfPattern);
            addPattern(dfPattern, FALSE, conflictingString, status);
        }
        // TODO Maybe we should return an error when the date format isn't simple.
        delete df;
        if (U_FAILURE(status)) { return; }

        df = DateFormat::createTimeInstance(style, locale);
        if (df != nullptr && (sdf = dynamic_cast<SimpleDateFormat*>(df)) != nullptr) {
            sdf->toPattern(dfPattern);
            addPattern(dfPattern, FALSE, conflictingString, status);

            // TODO: C++ and Java are inconsistent (see #12568).
            // C++ uses MEDIUM, but Java uses SHORT.
            if ( i==DateFormat::kShort && !dfPattern.isEmpty() ) {
                consumeShortTimePattern(dfPattern, status);
            }
        }
        // TODO Maybe we should return an error when the date format isn't simple.
        delete df;
        if (U_FAILURE(status)) { return; }
    }


// Source: dtptngen.cpp
// Lines 730-763
