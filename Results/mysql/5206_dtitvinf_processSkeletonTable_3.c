    void processSkeletonTable(const char *key, ResourceValue &value, UErrorCode &errorCode) {
        if (U_FAILURE(errorCode)) { return; }

        // Iterate over all the patterns in the current skeleton table
        const char *currentSkeleton = key;
        ResourceTable patternData = value.getTable(errorCode);
        if (U_FAILURE(errorCode)) { return; }
        for (int32_t k = 0; patternData.getKeyAndValue(k, key, value); k++) {
            if (value.getType() == URES_STRING) {
                // Process the key
                UCalendarDateFields calendarField = validateAndProcessPatternLetter(key);

                // If the calendar field has a valid value
                if (calendarField < UCAL_FIELD_COUNT) {
                    // Set the interval pattern
                    setIntervalPatternIfAbsent(currentSkeleton, calendarField, value, errorCode);
                    if (U_FAILURE(errorCode)) { return; }
                }
            }
        }
    }


// Source: dtitvinf.cpp
// Lines 286-306
