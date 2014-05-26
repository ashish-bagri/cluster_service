/**
 * The common main header file that combines backend cluster service and backend clone service
 * TODO: Should break it into 2 parts for clone service and cluster service
 *
 */
#ifndef INCLUDES
#define INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utilities_rt/string/string_utilities.h"
#include "../utilities_rt/ut/uthash.h"
#include "../utilities_rt/ut/utlist.h"
#include "../utilities_rt/ut/utstring.h"
#include "../utilities_rt/ut/utarray.h"
#include "../utilities_rt/aimath.h"
#include "../utilities_rt/compress/compress_b64.h"
#endif

#ifndef TYPES
#define TYPES
typedef int bool;
#define true 1
#define false 0
typedef unsigned int UNORM_V_TYPE;
#define LABEL_CLEAN 1
#define LABEL_MW -1
#endif

#include "bcs_data_struct.h"
#include "backend_clone_service.h"
#include "backend_cluster_service.h"
#include "backend_helper.h"
#include "bcs_common_methods.h"
#include "cleanup_helper.h"
