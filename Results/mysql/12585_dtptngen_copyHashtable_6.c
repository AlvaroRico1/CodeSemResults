DateTimePatternGenerator::copyHashtable(Hashtable *other, UErrorCode &status) {
    if (other == nullptr || U_FAILURE(status)) {
        return;
    }
    if (fAvailableFormatKeyHash != nullptr) {
        delete fAvailableFormatKeyHash;
        fAvailableFormatKeyHash = nullptr;
    }
    initHashtable(status);
    if(U_FAILURE(status)){
        return;
    }
    int32_t pos = UHASH_FIRST;
    const UHashElement* elem = nullptr;
    // walk through the hash table and create a deep clone
    while((elem = other->nextElement(pos))!= nullptr){
        const UHashTok otherKeyTok = elem->key;
        UnicodeString* otherKey = (UnicodeString*)otherKeyTok.pointer;
        fAvailableFormatKeyHash->puti(*otherKey, 1, status);
        if(U_FAILURE(status)){
            return;
        }
    }


// Source: dtptngen.cpp
// Lines 1694-1716
