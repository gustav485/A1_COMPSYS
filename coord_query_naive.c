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

struct naive_data {
  struct record *rs;
  int n;
};

struct naive_data* mk_naive(struct record* rs, int n) {
  // TODO Wrapper. index 
  struct naive_data* data = malloc(sizeof(struct naive_data));
  if (!data){
    printf(stderr,"Couldn't allocate memory for data");
    exit(1);
  }
  data->rs = rs;
  data->n = n;
  return data;
}

void free_naive(struct naive_data* data) {
  // TODO kald free på data
  free(data);
}

const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {
  // TODO for loop
  double BestDist = INFINITY;
  struct record *best = NULL;
  for (int i = 0; i < data->n; i++){
    double DifLon = (data->rs[i].lon - lon);
    double DifLat = (data ->rs[i].lat - lat);
    double Distance = (sqrt(pow(DifLon, 2)+pow(DifLat, 2)));

      if (Distance < BestDist) {
        BestDist = Distance;
        best = &data->rs[i];
      }
  }
  return best;
}
int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                    (mk_index_fn)mk_naive,
                    (free_index_fn)free_naive,
                    (lookup_fn)lookup_naive);
}
