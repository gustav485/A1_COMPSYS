#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "coord_query.h"
#include "timing.h"

/* Punkt i kd-træ */
struct Point {
    double coords[2];            // coords[0] = lon, coords[1] = lat
    const struct record *rec;    // peger på record
};

/* Node i kd-træ */
struct Node {
    struct Point point;
    int axis;          // 0 = lon, 1 = lat
    struct Node *left, *right;
};

/* KD-tree wrapper */
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

/* Byg kd-træ rekursivt */
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
    node->left = build(pts, mid, depth + 1);
    node->right = build(pts + mid + 1, n - mid - 1, depth + 1);

    return node;
}

/* Opret kd-træ fra records */
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

/* Frigør node rekursivt */
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

/* Wrapper til coord_query_loop */
const struct record* lookup_kdtree(struct kdtree_data *data, double lon, double lat) {
    const struct record *best = NULL;
    double best_dist = INFINITY;
    search(data->root, lon, lat, &best, &best_dist);
    return best;
}

/* Main til coord_query_loop */
int main(int argc, char** argv) {
    return coord_query_loop(argc, argv,
                            (mk_index_fn)mk_kdtree,
                            (free_index_fn)free_kdtree,
                            (lookup_fn)lookup_kdtree);
}
