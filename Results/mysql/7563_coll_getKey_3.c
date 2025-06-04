    virtual UObject* getKey(ICUServiceKey& key, UnicodeString* actualReturn, UErrorCode& status) const {
        UnicodeString ar;
        if (actualReturn == NULL) {
            actualReturn = &ar;
        }
        return (Collator*)ICULocaleService::getKey(key, actualReturn, status);
    }


// Source: coll.cpp
// Lines 182-188
