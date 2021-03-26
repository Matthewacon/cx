#pragma once

#ifdef CX_HEADER_ONLY
 #ifdef CX_HEADER_NAME
  #error \
   #CX_HEADER_NAME " is not available in header-only mode."
 #else
  #error \
   "This header is not available in header-only mode."
 #endif
#endif
