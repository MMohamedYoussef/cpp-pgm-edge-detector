# PGM Sobel edge detector

Convert an ASCII grayscale PGM image into a Sobel edge map with C++17 and the standard library. This is useful for inspecting simple image-processing pipelines without an image dependency.

```sh
c++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o edges
./edges fixtures/step.pgm example-out.pgm
python3 tests/test_cli.py ./edges
```

The program accepts `P2` PGM files with comments, dimensions up to 10,000 per side and at most 10 million pixels, and a declared maximum pixel value of 1–255. Each pixel must be present and within that range; extra data is rejected. Interior pixels use Sobel gradient magnitude, clamped to 255; border pixels are zero. Output is always `P2` with maximum 255. The demo creates [examples/edges.pgm](examples/edges.pgm) and prints `pixels=25`.

The output path must not already exist, including the input path. Output is staged and published only after a complete write, which requires hard-link support in the output directory. Bad or truncated images exit nonzero before creating output. The output is a diagnostic edge map, not a full photographic image pipeline.
