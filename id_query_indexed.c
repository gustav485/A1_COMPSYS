#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"
#include "id_query_naive.h"

struct index_record {
    int64_t osm_id;
    const struct record *record;
};
