initField(UnicodeString **field, int32_t& length, CalendarDataSink &sink, CharString &key, UErrorCode &status) {
    if (U_SUCCESS(status)) {
        UnicodeString keyUString(key.data(), -1, US_INV);
        UnicodeString* array = static_cast<UnicodeString*>(sink.arrays.get(keyUString));

        if (array != NULL) {
            length = sink.arraySizes.geti(keyUString);
            *field = array;
            // DateFormatSymbols takes ownership of the array:
            sink.arrays.remove(keyUString);
        } else {
            length = 0;
            status = U_MISSING_RESOURCE_ERROR;
        }
    }


// Source: dtfmtsym.cpp
// Lines 1873-1887
