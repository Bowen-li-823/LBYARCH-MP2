#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Assembly function.
   input  = array of float pixels in [0.0, 1.0]
   output = array of uint8_t pixels in [0, 255]
   count  = total number of pixels (width * height)
*/
void imgCvtGrayFloatToInt(const float *input, uint8_t *output, size_t count);

static uint8_t reference_pixel(float value)
{
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;

    /* Round to nearest integer. */
    return (uint8_t)(value * 255.0f + 0.5f);
}

static void reference_convert(const float *input, uint8_t *output, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        output[i] = reference_pixel(input[i]);
    }
}

static int check_correctness(const uint8_t *asm_output,
                             const uint8_t *reference_output,
                             size_t count)
{
    size_t errors = 0;

    for (size_t i = 0; i < count; i++) {
        if (asm_output[i] != reference_output[i]) {
            if (errors < 10) {
                printf("Mismatch at pixel %zu: ASM=%u, C=%u\n",
                       i,
                       (unsigned)asm_output[i],
                       (unsigned)reference_output[i]);
            }
            errors++;
        }
    }

    if (errors == 0) {
        printf("Correctness check: PASSED (%zu pixels)\n", count);
        return 1;
    }

    printf("Correctness check: FAILED (%zu mismatches)\n", errors);
    return 0;
}

static void print_image(const uint8_t *image, int height, int width)
{
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int index = row * width + col;
            printf("%3u", (unsigned)image[index]);
            if (col != width - 1) {
                printf(", ");
            }
        }
        printf("\n");
    }
}

static float random_pixel(void)
{
    return (float)rand() / (float)RAND_MAX;
}

#ifdef _WIN32
static double now_seconds(void)
{
    static LARGE_INTEGER frequency;
    static int initialized = 0;
    LARGE_INTEGER counter;

    if (!initialized) {
        QueryPerformanceFrequency(&frequency);
        initialized = 1;
    }

    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
}
#else
static double now_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}
#endif

static void benchmark_size(int width, int height, int runs)
{
    size_t count = (size_t)width * (size_t)height;
    float *input = (float *)malloc(count * sizeof(float));
    uint8_t *asm_output = (uint8_t *)malloc(count * sizeof(uint8_t));
    uint8_t *reference_output = (uint8_t *)malloc(count * sizeof(uint8_t));

    if (input == NULL || asm_output == NULL || reference_output == NULL) {
        fprintf(stderr, "Memory allocation failed for %dx%d image.\n", width, height);
        free(input);
        free(asm_output);
        free(reference_output);
        return;
    }

    for (size_t i = 0; i < count; i++) {
        input[i] = random_pixel();
    }

    reference_convert(input, reference_output, count);

    /* Warm-up run: avoids measuring first-call overhead. */
    imgCvtGrayFloatToInt(input, asm_output, count);

    double total_seconds = 0.0;
    double minimum_seconds = 1e100;

    for (int run = 0; run < runs; run++) {
        double start = now_seconds();
        imgCvtGrayFloatToInt(input, asm_output, count);
        double end = now_seconds();

        double elapsed = end - start;
        total_seconds += elapsed;
        if (elapsed < minimum_seconds) {
            minimum_seconds = elapsed;
        }
    }

    int correct = check_correctness(asm_output, reference_output, count);
    double average_seconds = total_seconds / (double)runs;
    double average_microseconds = average_seconds * 1000000.0;
    double minimum_microseconds = minimum_seconds * 1000000.0;
    double megapixels_per_second =
        average_seconds > 0.0 ? ((double)count / average_seconds) / 1000000.0 : 0.0;

    printf("Image size: %d x %d = %zu pixels\n", width, height, count);
    printf("Runs: %d\n", runs);
    printf("Average ASM time: %.3f microseconds\n", average_microseconds);
    printf("Minimum ASM time: %.3f microseconds\n", minimum_microseconds);
    printf("Average throughput: %.3f MPixels/second\n", megapixels_per_second);
    printf("Status: %s\n\n", correct ? "PASS" : "FAIL");

    free(input);
    free(asm_output);
    free(reference_output);
}

static void run_given_example(void)
{
    const int height = 3;
    const int width = 4;
    const size_t count = 12;

    float input[12] = {
        0.25f, 0.35f, 0.45f, 0.33f,
        0.55f, 0.65f, 0.75f, 0.33f,
        0.85f, 0.95f, 0.15f, 0.33f
    };

    uint8_t asm_output[12];
    uint8_t reference_output[12];

    imgCvtGrayFloatToInt(input, asm_output, count);
    reference_convert(input, reference_output, count);

    printf("=== Given 3 x 4 Example ===\n");
    print_image(asm_output, height, width);
    check_correctness(asm_output, reference_output, count);
    printf("\n");
}

static void run_manual_input(void)
{
    int height;
    int width;

    printf("Enter height and width: ");
    if (scanf("%d %d", &height, &width) != 2 || height <= 0 || width <= 0) {
        printf("Invalid height or width.\n");
        return;
    }

    size_t count = (size_t)height * (size_t)width;
    float *input = (float *)malloc(count * sizeof(float));
    uint8_t *output = (uint8_t *)malloc(count * sizeof(uint8_t));

    if (input == NULL || output == NULL) {
        printf("Memory allocation failed.\n");
        free(input);
        free(output);
        return;
    }

    printf("Enter %zu float pixel values from 0.0 to 1.0:\n", count);
    for (size_t i = 0; i < count; i++) {
        if (scanf("%f", &input[i]) != 1) {
            printf("Invalid pixel input.\n");
            free(input);
            free(output);
            return;
        }
    }

    imgCvtGrayFloatToInt(input, output, count);

    printf("\nInteger pixel values:\n");
    print_image(output, height, width);

    free(input);
    free(output);
}

int main(void)
{
    srand(12345); /* Fixed seed makes benchmark input repeatable. */

    run_given_example();

    printf("=== Performance Test: Assembly Function Only ===\n");
    benchmark_size(10, 10, 30);
    benchmark_size(100, 100, 30);
    benchmark_size(1000, 1000, 30);

    printf("=== Optional Manual Input ===\n");
    run_manual_input();

    return 0;
}
