CurrencyPluralInfo::deleteHash(Hashtable* hTable) {
    if ( hTable == nullptr ) {
        return;
    }
    int32_t pos = UHASH_FIRST;
    const UHashElement* element = nullptr;
    while ( (element = hTable->nextElement(pos)) != nullptr ) {
        const UHashTok valueTok = element->value;
        const UnicodeString* value = (UnicodeString*)valueTok.pointer;
        delete value;
    }


// Source: currpinf.cpp
// Lines 383-393
