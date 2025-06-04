void CharsetMatch::set(InputText *input, const CharsetRecognizer *cr, int32_t conf,
                       const char *csName, const char *lang)
{
    textIn = input;
    confidence = conf; 
    fCharsetName = csName;
    fLang = lang;
    if (cr != NULL) {
        if (fCharsetName == NULL) {
            fCharsetName = cr->getName();
        }


// Source: csmatch.cpp
// Lines 29-39
