# LBYARCH MP2
## Float Grayscale to uint8 Conversion Using C and x86-64 Assembly

---

Member : Li, Bowen | Fabregas, Matthew Drew

# Project Description

This project converts a grayscale image represented using single precision floating-point values (0.0 to 1.0) into unsigned 8-bit integer grayscale values (0 to 255).

The conversion is implemented using:

- C (main program)
- x86-64 Assembly (NASM)

The C program is responsible for:

- Reading user input
- Allocating memory
- Calling the assembly function
- Printing the output
- Correctness checking
- Performance testing

The x86-64 assembly function is responsible for converting every floating-point pixel into an unsigned 8-bit integer.

---

# Formula

```
Integer Pixel = round(Float Pixel × 255)
```

Example

| Float | Integer |
|-------|---------|
|0.00|0|
|0.25|64|
|0.50|128|
|0.75|191|
|1.00|255|

---

# Files

```
main.c
img_convert_win64.asm
README.md
result.exe
result.png
```

---

# Software Used

- MSYS2 UCRT64
- GCC
- NASM

---

# How to Compile

Download MSYS2

Open **MSYS2 UCRT64**

Go to the project directory.

Example

```bash
cd "/c/Users/User/OneDrive/Desktop/LBYARCH Mp2"
```

Assemble the assembly source

```bash
nasm -f win64 img_convert_win64.asm -o img_convert_win64.obj
```

Compile and link

```bash
gcc -O2 -Wall -Wextra -std=c11 main.c img_convert_win64.obj -o result.exe
```

---

# How to Run

Inside MSYS2

```bash
./result.exe
```

or inside Windows Command Prompt

```cmd
result.exe
```

---

# Example Input

```
3 4

0.25 0.35 0.45 0.33
0.55 0.65 0.75 0.33
0.85 0.95 0.15 0.33
```

---

# Example Output

```
64 89 115 84
140 166 191 84
217 242 38 84
```

---

# Correctness Check

The program compares the assembly output with the C reference implementation.

If every pixel matches, the program displays

```
Correctness check: PASSED
```

---

# Performance Test

The assembly function is timed only.

The following image sizes are tested.

- 10 × 10
- 100 × 100
- 1000 × 1000

Each size is executed 30 times.

The average execution time is reported.

---

# Scalar SIMD Instructions Used

The assembly implementation uses scalar SIMD instructions.

- MOVSS
- MULSS
- ADDSS
- MAXSS
- MINSS
- CVTTSS2SI

Registers used

- XMM0
- XMM1

---

# Performance Results

This section details the benchmark results of the x86-64 assembly conversion function. The performance was tested across three different image resolutions (10x10, 100x100, and 1000x1000). To ensure statistical reliability and account for timer overhead, each image size was processed for 30 consecutive runs. The table below summarizes the average execution time, minimum execution time, overall throughput, and the correctness status.

| Image Size | Avg ASM Time (μs) | Min ASM Time (μs) | Avg Throughput (MP/s) | Status |
|------------|-------------------|-------------------|-----------------------|--------|
| 10 × 10 | 0.103 | 0.100 | 967.771 | PASS |
| 100 × 100 | 15.057 | 8.400 | 664.158 | PASS |
| 1000 × 1000 | 1704.900 | 1034.000 | 586.545 | PASS |

### Output Screenshot

![Program Output](output_matt.png)

---

# Performance Analysis

The benchmark results reveal several key hardware and software characteristics regarding how the assembly code executes.

**1. Time Complexity and Scaling**
As expected, the execution time scales linearly with the number of pixels. Processing 100 pixels (10×10) took approximately 0.1 μs, while processing 1,000,000 pixels (1000×1000) took roughly 1,704 μs. Because the assembly function loops through the image one pixel at a time, the algorithm operates at a time complexity of **O(n)**, where 'n' is the total number of pixels. 

**2. Throughput Bottleneck (Cache vs. Main Memory)**
While the raw execution time scales linearly, the *Average Throughput* (MPixels/second) actually decreases as the image size increases. It drops from ~967 MP/s on the 10×10 image down to ~586 MP/s on the 1000×1000 image. This happens due to **CPU Cache limits**. Small images easily fit entirely inside the CPU's ultra-fast L1/L2 cache, meaning memory access is nearly instantaneous. However, a 1000×1000 image is 4 Megabytes of floating-point data. This spills out of the fast cache and forces the CPU to constantly fetch data from slower Main Memory (RAM), which introduces memory latency and slightly bottlenecks the overall calculation speed.

**3. Optimization Limits (Scalar vs Vector SIMD)**
While the code is highly optimized at the hardware level, it is fundamentally limited by the project instructions. The rubric specifically required the use of **Scalar SIMD** instructions (such as `MOVSS` and `MULSS`). This forces the CPU to process exactly one pixel per clock cycle. If we had been allowed to use Packed/Vector SIMD instructions, we could have processed 4 or 8 pixels simultaneously in a single instruction, which would have drastically improved the throughput.

---



---

# Video

()

