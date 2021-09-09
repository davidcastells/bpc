# Bit-Parallel Semi-Global Levenshtein Distance on FPGA

Banded approach of semi-global levenshtein distance on FPGA using OpenCL.

This work was published in FPL2021.

The [short] video presentation is available [here](https://www.youtube.com/watch?v=8FJtOtcGNpk&t=274s):

[![Youtube video](https://github.com/davidcastells/bpc/blob/master/video.png?raw=true)](https://www.youtube.com/watch?v=8FJtOtcGNpk&t=274s)



## OpenCL pre-alignment filters Test

This is a test application to test the effectiveness of our FPGA-based pre-alignment filter based on a bit parallel strategy using diagonal bit vectors. We provide the code we have used in our FPL2021 publication "as is". Improvements in the proposal will probably be uploaded to future repositories.


Prealignment filters are used to discard sequence pairs that have a higher number of errors than a given threshold.

The test main goal is to build synthetic sequences with a number of controlled errors to analyze the response of the filter in various situations.

The source code embedds the edlib (https://github.com/Martinsos/edlib/) to check the number of errors when necessary.

### Modes of operation

The application can work in two modes of operation.

-- Single Test
   In this mode the user provides the sequences to be compared

-- Multiple Synthetic Test
   In this mode the user provides the characteristics of the features that are randomly created. 

### Options

| Parameter | Description |
|-----|----------------|
| -v | verbose output |
| -pid <number> | Identifier of the OpenCL Platform to use |
| -tl <number> | Length of the Text |
| -pl <number> | Length of the pattern |
| -ES <number> | Number of Substitution Errors |
| -EI <number> | Number of Insertion Errors |
| -ED <number> | Number of Deletion Errors |
| -th <number> | The error threshold |
| -N <number> | The number of pairs to test |
| -t <string> | The text sequence for a single test |
| -p <string> | The pattern sequence for a single test |

### Examples

filter-filter -pid 0 -ES 4 -th 3 -tl 100 -pl 120 -n 10

It creates 10 sequence pairs. The text length and pattern length are all equal to 100. The pattern has 4 substitution errors compared with the text and the detection threshold is set to 3. The OpenCL platform 0 is used and the SHD algorithm is tested.
