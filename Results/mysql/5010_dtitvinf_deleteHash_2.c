DateIntervalInfo::deleteHash(Hashtable* hTable)
{
    if ( hTable == NULL ) {
        return;
    }
    int32_t pos = UHASH_FIRST;
    const UHashElement* element = NULL;
    while ( (element = hTable->nextElement(pos)) != NULL ) {
        const UHashTok valueTok = element->value;
        const UnicodeString* value = (UnicodeString*)valueTok.pointer;
        delete[] value;
    }


// Source: dtitvinf.cpp
// Lines 700-711
