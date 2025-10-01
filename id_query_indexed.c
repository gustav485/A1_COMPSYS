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

struct index_data {
    struct indexed_record *irs;
    int n;
}

struct indexed_data* mk_indexed(struct record* rs, int n);
void free_indexed(struct indexed_data* data);
const struct record* lookup_indexed(struct indexed_data *data, int64_t needle);

