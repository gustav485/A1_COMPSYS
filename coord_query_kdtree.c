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

struct Point {
    double coords[2];              // lon, lat
    const struct record *rec;      // peger på record
};

struct Node {
    struct Point point;
    int axis;                      // 0 = lon, 1 = lat
    struct Node *left, *right;
};

struct kdtree_data {
    struct Node *root;
};

/* Sammenlignere */
int compare_lon(const void *a, const void *b) {
    const struct Point *pa = a, *pb = b;
    return (pa->coords[0] > pb->coords[0]) - (pa->coords[0] < pb->coords[0]);
}
int compare_lat(const void *a, const void *b) {
    const struct Point *pa = a, *pb = b;
    return (pa->coords[1] > pb->coords[1]) - (pa->coords[1] < pb->coords[1]);
}

/* Byg rekursivt */
struct Node* build(struct Point *pts, int n, int depth) {
    if (n <= 0) return NULL;

    int axis = depth % 2;
    int mid = n / 2;

    if (axis == 0) qsort(pts, n, sizeof(struct Point), compare_lon);
    else qsort(pts, n, sizeof(struct Point), compare_lat);

    struct Node *node = malloc(sizeof(struct Node));
    assert(node);
    node->point = pts[mid];
    node->axis = axis;
    node->left  = build(pts, mid, depth + 1);
    node->right = build(pts + mid + 1, n - mid - 1, depth + 1);

    return node;
}

struct kdtree_data* mk_kdtree(struct record* rs, int n) {
    struct Point *pts = malloc(n * sizeof(struct Point));
    assert(pts);

    for (int i = 0; i < n; i++) {
        pts[i].coords[0] = rs[i].lon;
        pts[i].coords[1] = rs[i].lat;
        pts[i].rec = &rs[i];
    }

    struct kdtree_data *data = malloc(sizeof(struct kdtree_data));
    assert(data);
    data->root = build(pts, n, 0);

    free(pts);
    return data;
}

void free_nodes(struct Node *node) {
    if (!node) return;
    free_nodes(node->left);
    free_nodes(node->right);
    free(node);
}

void free_kdtree(struct kdtree_data* data) {
    free_nodes(data->root);
    free(data);
}

/* Nærmeste nabo søgning */
void search(const struct Node *node, double lon, double lat,
            const struct record **best, double *best_dist) {
    if (!node) return;

    double dx = node->point.coords[0] - lon;
    double dy = node->point.coords[1] - lat;
    double dist = sqrt(dx*dx + dy*dy);

    if (dist < *best_dist) {
        *best_dist = dist;
        *best = node->point.rec;
    }

    double diff = (node->axis == 0 ? lon - node->point.coords[0]
                                   : lat - node->point.coords[1]);

    if (diff < 0) {
        search(node->left, lon, lat, best, best_dist);
        if (fabs(diff) < *best_dist) search(node->right, lon, lat, best, best_dist);
    } else {
        search(node->right, lon, lat, best, best_dist);
        if (fabs(diff) < *best_dist) search(node->left, lon, lat, best, best_dist);
    }
}

const struct record* lookup_kdtree(struct kdtree_data *data, double lon, double lat) {
    const struct record *best = NULL;
    double best_dist = INFINITY;
    search(data->root, lon, lat, &best, &best_dist);
    return best;
}

int main(int argc, char** argv) {
    return coord_query_loop(argc, argv,
                            (mk_index_fn)mk_kdtree,
                            (free_index_fn)free_kdtree,
                            (lookup_fn)lookup_kdtree);
}
