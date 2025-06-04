DateIntervalFormat::format(const Formattable& obj,
                           UnicodeString& appendTo,
                           FieldPosition& fieldPosition,
                           UErrorCode& status) const {
    if ( U_FAILURE(status) ) {
        return appendTo;
    }

    if ( obj.getType() == Formattable::kObject ) {
        const UObject* formatObj = obj.getObject();
        const DateInterval* interval = dynamic_cast<const DateInterval*>(formatObj);
        if (interval != NULL) {
            return format(interval, appendTo, fieldPosition, status);
        }
    }


// Source: dtitvfmt.cpp
// Lines 258-272
