#include <stdlib.h>

float generate_random_value(float min, float max) {
    return ((float)rand() / (float)RAND_MAX) * (max - min) + min;
}