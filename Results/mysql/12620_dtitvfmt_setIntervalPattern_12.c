DateIntervalFormat::setIntervalPattern(UCalendarDateFields field,
                                       const UnicodeString& intervalPattern,
                                       UBool laterDateFirst) {
    const UnicodeString* pattern = &intervalPattern;
    UBool order = laterDateFirst;
    // check for "latestFirst:" or "earliestFirst:" prefix
    int8_t prefixLength = UPRV_LENGTHOF(gLaterFirstPrefix);
    int8_t earliestFirstLength = UPRV_LENGTHOF(gEarlierFirstPrefix);
    UnicodeString realPattern;
    if ( intervalPattern.startsWith(gLaterFirstPrefix, prefixLength) ) {
        order = true;
        intervalPattern.extract(prefixLength,
                                intervalPattern.length() - prefixLength,
                                realPattern);
        pattern = &realPattern;
    } else if ( intervalPattern.startsWith(gEarlierFirstPrefix,
                                           earliestFirstLength) ) {
        order = false;
        intervalPattern.extract(earliestFirstLength,
                                intervalPattern.length() - earliestFirstLength,
                                realPattern);
        pattern = &realPattern;
    }

    int32_t splitPoint = splitPatternInto2Part(*pattern);

    UnicodeString firstPart;
    UnicodeString secondPart;
    pattern->extract(0, splitPoint, firstPart);
    if ( splitPoint < pattern->length() ) {
        pattern->extract(splitPoint, pattern->length()-splitPoint, secondPart);
    }
    setPatternInfo(field, &firstPart, &secondPart, order);
}


// Source: dtitvfmt.cpp
// Lines 1195-1228
