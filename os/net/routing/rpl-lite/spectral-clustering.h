#include <math.h>
#define MAX_NODES 100
#define EPSILON 1e-9

void spectralClustering(int, double adjMatrix[MAX_NODES][MAX_NODES], int k, int clusters[]);
