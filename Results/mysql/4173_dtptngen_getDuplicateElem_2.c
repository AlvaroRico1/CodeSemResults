PatternMap::getDuplicateElem(
            const UnicodeString &basePattern,
            const PtnSkeleton &skeleton,
            PtnElem *baseElem) {
   PtnElem *curElem;

   if ( baseElem == nullptr ) {
         return nullptr;
   }
   else {
         curElem = baseElem;
   }
   do {
     if ( basePattern.compare(curElem->basePattern)==0 ) {
         UBool isEqual = TRUE;
         for (int32_t i = 0; i < UDATPG_FIELD_COUNT; ++i) {
            if (curElem->skeleton->type[i] != skeleton.type[i] ) {
                isEqual = FALSE;
                break;
            }
        }
        if (isEqual) {
            return curElem;
        }
     }
     curElem = curElem->next.getAlias();
   } while( curElem != nullptr );

   // end of the list
   return nullptr;

}  // PatternMap::getDuplicateElem


// Source: dtptngen.cpp
// Lines 2079-2110
