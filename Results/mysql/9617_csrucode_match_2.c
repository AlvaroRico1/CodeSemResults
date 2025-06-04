UBool CharsetRecog_UTF_16_LE::match(InputText* textIn, CharsetMatch *results) const
{
    const uint8_t *input = textIn->fRawInput;
    int32_t confidence = 10;
    int32_t length = textIn->fRawLength;

    int32_t bytesToCheck = (length > 30) ? 30 : length;
    for (int32_t charIndex=0; charIndex<bytesToCheck-1; charIndex+=2) {
        UChar codeUnit = input[charIndex] | (input[charIndex + 1] << 8);
        if (charIndex == 0 && codeUnit == 0xFEFF) {
            confidence = 100;     // UTF-16 BOM
            if (length >= 4 && input[2] == 0 && input[3] == 0) {
                confidence = 0;   // UTF-32 BOM
            }
            break;
        }
        confidence = adjustConfidence(codeUnit, confidence);
        if (confidence == 0 || confidence == 100) {
            break;
        }
    }
    if (bytesToCheck < 4 && confidence < 100) {
        confidence = 0;
    }
    results->set(textIn, this, confidence);
    return (confidence > 0);
}


// Source: csrucode.cpp
// Lines 90-116
