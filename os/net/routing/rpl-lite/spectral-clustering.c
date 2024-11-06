#ifdef SPECTRAL
#include "net/routing/rpl-lite/spectral-clustering.h"

void computeDegreeMatrix(int n, float adjMatrix[MAX_NODES][MAX_NODES], float degreeMatrix[MAX_NODES][MAX_NODES]) {
    for (int i = 0; i < n; i++) {
        float degree = 0.0;
        for (int j = 0; j < n; j++) {
            degree += adjMatrix[i][j];
        }
        degreeMatrix[i][i] = degree;
    }
}

void computeLaplacian(int n, float adjMatrix[MAX_NODES][MAX_NODES], float laplacian[MAX_NODES][MAX_NODES]) {
    float degreeMatrix[MAX_NODES][MAX_NODES] = {0};
    computeDegreeMatrix(n, adjMatrix, degreeMatrix);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            laplacian[i][j] = degreeMatrix[i][j] - adjMatrix[i][j];
        }
    }
}

void matrixMultiply(int n, float A[MAX_NODES][MAX_NODES], float B[MAX_NODES][MAX_NODES], float C[MAX_NODES][MAX_NODES]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

float powerMethod(int n, float matrix[MAX_NODES][MAX_NODES], float eigenVector[MAX_NODES]) {
    float temp[MAX_NODES];
    for (int i = 0; i < n; i++) {
        eigenVector[i] = 1.0;
    }
    float eigenValue = 0.0;
    for (int iter = 0; iter < 100; iter++) {
        float norm = 0.0;
        for (int i = 0; i < n; i++) {
            temp[i] = 0.0;
            for (int j = 0; j < n; j++) {
                temp[i] += matrix[i][j] * eigenVector[j];
            }
            norm += temp[i] * temp[i];
        }
        norm = sqrt(norm);
        for (int i = 0; i < n; i++) {
            temp[i] /= norm;
        }
        float newEigenValue = 0.0;
        for (int i = 0; i < n; i++) {
            newEigenValue += temp[i] * eigenVector[i];
        }
        if (fabs(newEigenValue - eigenValue) < EPSILON) {
            break;
        }
        eigenValue = newEigenValue;
        for (int i = 0; i < n; i++) {
            eigenVector[i] = temp[i];
        }
    }
    return eigenValue;
}

void kMeansClustering(int n, int k, float eigenVectors[MAX_NODES][MAX_NODES], int clusters[MAX_NODES]) {
    float centroids[k][MAX_NODES];
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < n; j++) {
            centroids[i][j] = eigenVectors[i][j];
        }
    }
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < n; i++) {
            float minDist = INFINITY;
            int minIndex = -1;
            for (int j = 0; j < k; j++) {
                float dist = 0.0;
                for (int l = 0; l < n; l++) {
                    dist += (eigenVectors[i][l] - centroids[j][l]) * (eigenVectors[i][l] - centroids[j][l]);
                }
                if (dist < minDist) {
                    minDist = dist;
                    minIndex = j;
                }
            }
            if (clusters[i] != minIndex) {
                changed = 1;
                clusters[i] = minIndex;
            }
        }
        for (int i = 0; i < k; i++) {
            for (int j = 0; j < n; j++) {
                centroids[i][j] = 0.0;
            }
        }
        int count[k];
        for (int i = 0; i < k; i++) {
            count[i] = 0;
        }
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                centroids[clusters[i]][j] += eigenVectors[i][j];
            }
            count[clusters[i]]++;
        }
        for (int i = 0; i < k; i++) {
            for (int j = 0; j < n; j++) {
                if (count[i] > 0) {
                    centroids[i][j] /= count[i];
                }
            }
        }
    }
}

void spectralClustering(int n, float adjMatrix[MAX_NODES][MAX_NODES], int k, int clusters[MAX_NODES]) {
    float laplacian[MAX_NODES][MAX_NODES] = {0};
    computeLaplacian(n, adjMatrix, laplacian);

    float eigenVectors[MAX_NODES][MAX_NODES] = {0};
    for (int i = 0; i < k; i++) {
        eigenVectors[i][0] = powerMethod(n, laplacian, eigenVectors[i]);
    }

    kMeansClustering(n, k, eigenVectors, clusters);
}


void createAdjacencyMatrix(uip_sr_node_t * head, int n, int adjMatrix[MAX_NODES][MAX_NODES]) {
    // Initialize the adjacency matrix with 0
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            adjMatrix[i][j] = 0;
        }
    }

    // Traverse the linked list and update the adjacency matrix
    uip_sr_node_t *link;
    while (current != NULL) {
        int src = current->src;
        int dest = current->dest;
        adjMatrix[src][dest] = 1;
        adjMatrix[dest][src] = 1;  // For undirected graphs, mark both (src, dest) and (dest, src)
        link = uip_sr_node_next(link);
    }
}

#endif