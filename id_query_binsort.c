#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct index_record {
    int64_t osm_id;
    const struct record *record;
};

struct indexed_data {
    struct index_record *irs;
    int n;
};
void quickSort(struct index_record arr[], int low, int high);

struct indexed_data* mk_sorted(struct record* rs, int n) {
    struct indexed_data* data = malloc(sizeof(struct indexed_data));

    //Skal lave plads til n index records
    struct index_record* irs = malloc(n * sizeof(struct index_record));   
    if (!data){
        fprintf(stderr, "Couldn't allocate memory for data");
        exit(1);
    }

    data->irs = irs;
    data->n = n;

    for (int i = 0; i < data->n; i++){
        data->irs[i].osm_id = rs[i].osm_id;
        data->irs[i].record = &rs[i];
    }
    quickSort(data->irs, 0, n - 1);
    return data;
}

void free_sorted(struct indexed_data* data) {
    free(data->irs);
    free(data);
}


void swap(struct index_record *a, struct index_record *b) {
    struct index_record temp = *a;
    *a = *b;
    *b = temp;
}

int partition(struct index_record arr[], int low, int high) {
    int64_t pivot = arr[high].osm_id; 
    int i = low - 1;

    for (int j = low; j <= high - 1; j++) {
        if (arr[j].osm_id < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return i + 1;
}

void quickSort(struct index_record arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);

        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}


const struct record* binarySearch(struct indexed_data *data, int64_t needle) {
    int64_t low = 0;
    int64_t high = data->n - 1;
    if (high < low) {
        return NULL;
    }

    while (low <= high) {
        int64_t mid = low + (high - low) / 2;

        if (data->irs[mid].osm_id == needle) {
            return data->irs[mid].record;
        }
        else if (data->irs[mid].osm_id < needle) {
            low = mid + 1;
        }
        else {
            high = mid - 1;
        }
    }
    return NULL;
}


int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_sorted,
                    (free_index_fn)free_sorted,
                    (lookup_fn)binarySearch);
}