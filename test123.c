#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "coord_query.h"

struct kdtree_data {
  struct record *rs;
  int n;
};

struct Point {
    double coords[2];
};

struct Node {
    struct Point point;
    int axis;
    struct Node *left, *right;
};

struct kdtree_data* mk_kdtree(struct record* rs, int n) {
    struct kdtree_data *data = malloc(sizeof(struct kdtree_data));
    struct Point *point = malloc(sizeof(struct Point));
    struct Node *node = malloc(sizeof(struct Node));
    if (!data){
        printf(stderr,"Couldn't allocate memory for data");
        exit(1);
    }

    data->rs = rs;
    data->n = n;
    return data;
}

void free_kdtree(struct kdtree_data* data, struct Point* point, struct Node *node) {
    free(data);
    free(point);
    free(node);
}

const struct record* lookup_kdtree(struct kdtree_data *data, double lon, double lat) {
    
}
int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                    (mk_index_fn)mk_kdtree,
                    (free_index_fn)free_kdtree,
                    (lookup_fn)lookup_kdtree);
}
