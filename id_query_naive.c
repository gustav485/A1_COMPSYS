#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

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

const struct record* lookup_naive(struct naive_data *data, int64_t needle) {
  // TODO for loop
  for (int i = 0; i < data->n; i++){
    if (data->rs[i].osm_id == needle){
      return &data->rs[i];
    }
  }
  return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_naive,
                    (free_index_fn)free_naive,
                    (lookup_fn)lookup_naive);
}


//til longitude og latitude
//Bruge branch if a > b return x hvor x kan være 1, 0, -1

//rs er et array med records 
//n er antallet af hvor mange der er