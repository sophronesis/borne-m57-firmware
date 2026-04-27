// custom rgb matrix effects for the borne layout.
// the effect implementation lives in borne_status.h so the build system
// doesn't pick it up as a stray translation unit.

RGB_MATRIX_EFFECT(borne_status)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#    include "borne_status.h"
#endif
