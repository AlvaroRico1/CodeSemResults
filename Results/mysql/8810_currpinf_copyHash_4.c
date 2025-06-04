CurrencyPluralInfo::copyHash(const Hashtable* source,
                           Hashtable* target,
                           UErrorCode& status) {
    if (U_FAILURE(status)) {
        return;
    }
    int32_t pos = UHASH_FIRST;
    const UHashElement* element = nullptr;
    if (source) {
        while ( (element = source->nextElement(pos)) != nullptr ) {
            const UHashTok keyTok = element->key;
            const UnicodeString* key = (UnicodeString*)keyTok.pointer;
            const UHashTok valueTok = element->value;
            const UnicodeString* value = (UnicodeString*)valueTok.pointer;
            LocalPointer<UnicodeString> copy(new UnicodeString(*value), status);
            if (U_FAILURE(status)) {
                return;
            }
            // The HashTable owns the 'copy' object after the call to put().
            target->put(UnicodeString(*key), copy.orphan(), status);
            if (U_FAILURE(status)) {
                return;
            }
        }


// Source: currpinf.cpp
// Lines 412-435
