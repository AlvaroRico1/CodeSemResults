DateIntervalInfo::initializeData(const Locale& locale, UErrorCode& status)
{
    fIntervalPatterns = initHash(status);
    if (U_FAILURE(status)) {
      return;
    }
    const char *locName = locale.getName();

    // Get the correct calendar type
    const char * calendarTypeToUse = gGregorianTag; // initial default
    char         calendarType[ULOC_KEYWORDS_CAPACITY]; // to be filled in with the type to use, if all goes well
    char         localeWithCalendarKey[ULOC_LOCALE_IDENTIFIER_CAPACITY];
    // obtain a locale that always has the calendar key value that should be used
    (void)ures_getFunctionalEquivalent(localeWithCalendarKey, ULOC_LOCALE_IDENTIFIER_CAPACITY, NULL,
                                     "calendar", "calendar", locName, NULL, FALSE, &status);
    localeWithCalendarKey[ULOC_LOCALE_IDENTIFIER_CAPACITY-1] = 0; // ensure null termination
    // now get the calendar key value from that locale
    int32_t calendarTypeLen = uloc_getKeywordValue(localeWithCalendarKey, "calendar", calendarType,
                                                   ULOC_KEYWORDS_CAPACITY, &status);
    if (U_SUCCESS(status) && calendarTypeLen < ULOC_KEYWORDS_CAPACITY) {
        calendarTypeToUse = calendarType;
    }
    status = U_ZERO_ERROR;

    // Instantiate the resource bundles
    UResourceBundle *rb, *calBundle;
    rb = ures_open(NULL, locName, &status);
    if (U_FAILURE(status)) {
        return;
    }
    calBundle = ures_getByKeyWithFallback(rb, gCalendarTag, NULL, &status);


    if (U_SUCCESS(status)) {
        UResourceBundle *calTypeBundle, *itvDtPtnResource;

        // Get the fallback pattern
        const UChar* resStr;
        int32_t resStrLen = 0;
        calTypeBundle = ures_getByKeyWithFallback(calBundle, calendarTypeToUse, NULL, &status);
        itvDtPtnResource = ures_getByKeyWithFallback(calTypeBundle,
                                                     gIntervalDateTimePatternTag, NULL, &status);
        resStr = ures_getStringByKeyWithFallback(itvDtPtnResource, gFallbackPatternTag,
                                                 &resStrLen, &status);
        if ( U_SUCCESS(status) ) {
            UnicodeString pattern = UnicodeString(TRUE, resStr, resStrLen);
            setFallbackIntervalPattern(pattern, status);
        }
        ures_close(itvDtPtnResource);
        ures_close(calTypeBundle);


        // Instantiate the sink
        DateIntervalSink sink(*this, calendarTypeToUse);
        const UnicodeString &calendarTypeToUseUString = sink.getNextCalendarType();

        // Already loaded calendar types
        Hashtable loadedCalendarTypes(FALSE, status);

        if (U_SUCCESS(status)) {
            while (!calendarTypeToUseUString.isBogus()) {
                // Set an error when a loop is detected
                if (loadedCalendarTypes.geti(calendarTypeToUseUString) == 1) {
                    status = U_INVALID_FORMAT_ERROR;
                    break;
                }

                // Register the calendar type to avoid loops
                loadedCalendarTypes.puti(calendarTypeToUseUString, 1, status);
                if (U_FAILURE(status)) { break; }

                // Get the calendar string
                CharString calTypeBuffer;
                calTypeBuffer.appendInvariantChars(calendarTypeToUseUString, status);
                if (U_FAILURE(status)) { break; }
                const char *calType = calTypeBuffer.data();

                // Reset the next calendar type to load.
                sink.resetNextCalendarType();

                // Get all resources for this calendar type
                ures_getAllItemsWithFallback(calBundle, calType, sink, status);
            }
        }
    }

    // Close the opened resource bundles
    ures_close(calBundle);
    ures_close(rb);
}


// Source: dtitvinf.cpp
// Lines 387-476
