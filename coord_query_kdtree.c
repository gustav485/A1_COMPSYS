#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "coord_query.h"
#include "timing.h"

struct Point {
    double coords[2];           //Array med to koordinater
    const struct record *rec;   //
};

struct Node {
    struct Point point;         // Punkt 
    int axis;                   // Dimension af. 0 for x , 1 for y.
    struct Node *left;          // pointer til venstre barn
    struct Node *right;         // Pointer til højre barn
};

struct kdtree_data {        
    struct Node *root;          //pointer til træets rod
};

int compare_lon(const void *a, const void *b) {         // Sammenligner to koordinater
    const struct Point *pa = a, *pb = b;
    return (pa->coords[0] > pb->coords[0]) - (pa->coords[0] < pb->coords[0]);       // Returnerer -1, 0 eller 1 alt efter om pa < pb, pa = pb eller pa > pb
}

int compare_lat(const void *a, const void *b) {         //Sammenligner to koordinater
    const struct Point *pa = a, *pb = b;
    return (pa->coords[1] > pb->coords[1]) - (pa->coords[1] < pb->coords[1]);
}

struct Node* build(struct Point *pts, int n, int depth) {       // Opbygning af kd træ
    if (n <= 0) return NULL;        //basecase

    int axis = depth % 2;           // bruges til at skifte imellem lon og lat
    int mid = n / 2;                //Finder medianen 

    if (axis == 0) qsort(pts, n, sizeof(struct Point), compare_lon);        //Sortering af punkterne alt efter om det skal være på x eller y.
    else qsort(pts, n, sizeof(struct Point), compare_lat);

    struct Node *node = malloc(sizeof(struct Node));                //Allokere plads til træet
    node->point = pts[mid];                                 // median punktet bliver til nodens punkt
    node->axis = axis;                                      //Gemmer hvilken akse der skal bruges til sortering
    node->left = build(pts, mid, depth + 1);                    //Bygger venstre barn-træ (alt som er mindre end medianen)
    node->right = build(pts + mid + 1, n - mid - 1, depth + 1); //Bygger højre barn-træ (alt som er større end medianen)

    return node;
}

struct kdtree_data* mk_kdtree(struct record* rs, int n) {
    struct Point *pts = malloc(n * sizeof(struct Point));       //Allokerer hukommelse til et array er holder vores punkter                

    for (int i = 0; i < n; i++) {                       //Lægger data fra records til vores array rs
        pts[i].coords[0] = rs[i].lon;
        pts[i].coords[1] = rs[i].lat;
        pts[i].rec = &rs[i];
    }

    struct kdtree_data *data = malloc(sizeof(struct kdtree_data));      //Allokerer hukommelse til hele træet
    data->root = build(pts, n, 0);                          //Bygger træet.

    free(pts);                  //Frigiver vores allokeret plads.
    return data;            //Returnerer pointer til kd træet i hukommelsen.
}

/* Frigør node rekursivt */
void free_nodes(struct Node *node) {        // Frigiver den allokeret plads efter brug
    if (!node) return;
    free_nodes(node->left);
    free_nodes(node->right);
    free(node);
}

void free_kdtree(struct kdtree_data* data) {        // frigiver allokeret plads efter brug
    free_nodes(data->root);
    free(data);
}

//Søge funktion til at finde nærmeste punkt i træet. 
void search(const struct Node *node, double lon, double lat, const struct record **best, double *best_dist) {
    if (!node) return;      //Base case

    //Afstands beregninger fra searchpoint til nodens punkt.
    double dx = node->point.coords[0] - lon;        //forskel i lon                       
    double dy = node->point.coords[1] - lat;        //forskel i lat
    double dist = sqrt(dx*dx + dy*dy);          //Euklid distance

    if (dist < *best_dist) {            //Opdatere bedste punkt hvis det aktive punkt er tættere på.
        *best_dist = dist;
        *best = node->point.rec;
    }

    double diff = (node->axis == 0 ? lon - node->point.coords[0]: lat - node->point.coords[1]); 

    if (diff < 0) {
        search(node->left, lon, lat, best, best_dist);
        if (fabs(diff) < *best_dist) search(node->right, lon, lat, best, best_dist);            //Søger på begge side af punktet 
    } else {
        search(node->right, lon, lat, best, best_dist);
        if (fabs(diff) < *best_dist) search(node->left, lon, lat, best, best_dist);
    }
}

const struct record* lookup_kdtree(struct kdtree_data *data, double lon, double lat) {          //
    const struct record *best = NULL;           //pointer til bedst nærmeste punkt
    double best_dist = INFINITY;                
    search(data->root, lon, lat, &best, &best_dist);        //søger efter nærmeste rod
    return best;                                        //Returnerer den nærmeste rod.
}

int main(int argc, char** argv) {
    return coord_query_loop(argc, argv,
                            (mk_index_fn)mk_kdtree,
                            (free_index_fn)free_kdtree,
                            (lookup_fn)lookup_kdtree);
}
