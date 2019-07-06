#ifndef GLOVEBOX_H
#define GLOVEBOX_H

#ifndef PLAINTEXT
#error PLAINTEXT must be defined (either 0 or 1).
#endif
#ifndef GB_SERVER
#error GB_SERVER must be defined (either 0 or 1).
#endif

#include "types/type_ids.h"

#include "flow_control.h"
#include "parallel.h"
#include "primitives.h"
#include "structhelper.h"
#include "tfhe.h"

#include "types/fixed.h"
#include "types/int.h"
#include "types/polynomial.h"

#include "types/array.h"
#include "types/string.h"

#endif // GLOVEBOX_H