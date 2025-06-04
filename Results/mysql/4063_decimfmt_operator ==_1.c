UBool DecimalFormat::operator==(const Format& other) const {
    auto* otherDF = dynamic_cast<const DecimalFormat*>(&other);
    if (otherDF == nullptr) {
        return false;
    }
    // If either object is in an invalid state, prevent dereferencing nullptr below.
    // Additionally, invalid objects should not be considered equal to anything.
    if (fields == nullptr || otherDF->fields == nullptr) {
        return false;
    }
    return fields->properties == otherDF->fields->properties && *fields->symbols == *otherDF->fields->symbols;
}


// Source: decimfmt.cpp
// Lines 500-511
