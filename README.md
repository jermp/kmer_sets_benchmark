[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.17582116.svg)](https://doi.org/10.5281/zenodo.17582116)

# Kmer sets benchmark

This is a benchmark of data structures for kmer dictionaries,
i.e., data structures that represent a set of kmers and support, at least,
exact membership queries. The goal of the benchmark is to test the
data structures using the same benchmarking code and methodology
(as well as, obviously, the same datasets and queries).

The scripts to run the benchmark are in the [`script`](script) folder.

Clone and compile as follows.

	git clone --recursive https://github.com/jermp/kmer_sets_benchmark.git
	cd kmer_sets_benchmark/
	mkdir build
	cd build/
	cmake .. -DUSE_MAX_KMER_LENGTH_63=Off
	make -j

Optionally, compile with `cmake .. -DUSE_MAX_KMER_LENGTH_63=On` to use k=63 in the benchmark.

### Tested dictionaries

The dictionaries benchmarked here are:

- [SSHash](https://github.com/jermp/sshash/tree/bench),
- [SBWT](https://github.com/algbio/SBWT),
- and [FMSI](https://github.com/OndrejSladky/fmsi).

All C++ implementations are by the respective authors.

### Datasets

For these benchmarks we used the datasets available here
[https://zenodo.org/records/17582116](https://zenodo.org/records/17582116): the files `*.eulertigs.fa.gz` were used as input, the `*.fastq.gz` files were used as queries instead.

| Collection | Num. distinct 31-mers | Num. distinct 63-mers |
|------------|----------------------:|----------------------:|
| Cod        |           502,465,200 |           556,585,658 |
| Kestrel    |         1,150,399,205 |         1,155,250,667 |
| Human      |         2,505,678,680 |         2,771,316,093 |
| NCBI-virus |           376,205,185 |           412,515,880 |
| SE         |           894,310,084 |         1,524,904,156 |
| HPRC       |         3,718,120,949 |         5,926,785,469 |

### Methodology

The dictionaries were built with a max RAM usage of 16 GB and 64 threads.
All queries were run using 1 thread, instead.

For SSHash and SBWT, the building time reported in the tables refers to the time it takes to index the `eulertigs.fa.gz` files. FMSI first requires the computation of masked super strings from the `eulertigs.fa.gz` files. We excluded this time from the building time and report only the time FMSI takes to index its computed super strings.

The space reported is the space taken by the dictionaries on disk.

Positive random lookup time was measured by querying 1 million kmers that appear in the dictionaries, half of which were reverse complemented to test the dictionaries in the most general case.

For negative random lookups, random kmers were generated (i.e., each nucleotide was uniformly sampled from {A,C,G,T}) and used as queries instead.

For random access, we uniformly generated 1 million ranks and retrieved the corresponding kmers.

Lastly, for streaming queries, we queried the dictionaries using FASTQ reads. For each dictionary, a FASTQ readset was chosen to have a high-hit workload (i.e., most kmers are found in the dictionaries).
See the folder [`script`](script) for details.

### Results

These are the results obtained on Nov 2025 (see logs [here](results))
on a machine equipped with an AMD Ryzen Threadripper PRO 7985WX processor clocked at 5.40GHz.
The code was compiled with `gcc` 13.3.0.


SSHash indexes reported here were built with option `--canonical`, using the indicated value for the `m` parameter (minimizer length).
All results are available here [https://github.com/jermp/sshash/tree/bench/benchmarks](https://github.com/jermp/sshash/tree/bench/benchmarks).

| k  | Collection | m  | Space (bits/kmer) | Space (total GB) | Building time (m:ss) | Positive random lookup (µs/kmer) | Negative random lookup (µs/kmer) | Random Access (µs/kmer) | Streaming Lookup high-hit (ns/kmer) |
|----|------------|:--:|:-----------------:|:----------------:|:--------------------:|:--------------------------------:|:--------------------------------:|:-----------------------:|:-----------------------------------:|
||
| 31 | Cod        | 20 |  9.01             | 0.57             | 0:26                 | 0.44                             | 0.37                             | 0.28                    |  30                                 |
|    | Kestrel    | 20 |  8.67             | 1.25             | 1:06                 | 0.44                             | 0.40                             | 0.28                    |  49                                 |
|    | Human      | 21 | 10.01             | 3.14             | 3:10                 | 0.61                             | 0.42                             | 0.35                    |  86                                 |
|    | NCBI-virus | 19 |  8.48             | 0.40             | 0:16                 | 0.41                             | 0.36                             | 0.26                    |  29                                 |
|    | SE         | 21 | 11.51             | 1.29             | 1:06                 | 0.70                             | 0.40                             | 0.36                    | 231                                 |
|    | HPRC       | 21 | 11.93             | 5.54             | 4:45                 | 0.80                             | 0.46                             | 0.54                    | 109                                 |
||
| 63 | Cod        | 24 |  4.9              | 0.35             | 0:15                 | 0.56                             | 0.45                             | 0.29                    |  66                                 |
|    | Kestrel    | 24 |  4.22             | 0.61             | 0:19                 | 0.54                             | 0.48                             | 0.33                    |  70                                 |
|    | Human      | 25 |  5.31             | 1.84             | 1:09                 | 0.69                             | 0.52                             | 0.36                    | 148                                 |
|    | NCBI-virus | 23 |  4.46             | 0.23             | 0:07                 | 0.52                             | 0.44                             | 0.28                    |  78                                 |
|    | SE         | 31 |  7.77             | 1.48             | 0:58                 | 1.28                             | 0.51                             | 0.41                    | 578                                 |
|    | HPRC       | 31 |  8.14             | 6.03             | 4:13                 | 1.11                             | 0.58                             | 0.64                    | 187                                 |

<p align="center"><b>Tab. 1 SSHash results</b></p>


SBWT indexes were all built with option `--add-reverse-complements` so that queries
return the same results as for the other indexes.

| k  | Collection | Space (bits/kmer) | Space (total GB) | Building time (m:ss) | Positive random Lookup (µs/kmer) | Negative random Lookup (µs/kmer) | Random Access (µs/kmer) | Streaming Lookup high-hit (ns/kmer) |
|----|------------|:-----------------:|:----------------:|:--------------------:|:--------------------------------:|:--------------------------------:|:-----------------------:|:-----------------------------------:|
||
| 31 | Cod        | 10.52             | 0.66             |  03:34               | 2.72                             | 0.91                             |  7.71                   |  62                                 |
|    | Kestrel    | 10.52             | 1.51             |  07:57               | 2.87                             | 0.96                             |  9.48                   | 287                                 |
|    | Human      | 10.50             | 3.29             |  17:56               | 2.97                             | 1.07                             | 10.81                   | 266                                 |
|    | NCBI-virus | 10.53             | 0.50             |  02:44               | 2.71                             | 0.89                             |  6.96                   | 139                                 |
|    | SE         | 10.72             | 1.20             |  06:54               | 2.83                             | 0.97                             |  8.97                   | 189                                 |
|    | HPRC       | 10.50             | 4.88             |  29:24               | 3.12                             | 1.16                             | 11.45                   | 263                                 |
||
| 63 | Cod        | 10.52             | 0.73             |  06:13               | 6.59                             | 0.92                             | 16.05                   | 118                                 |
|    | Kestrel    | 10.55             | 1.52             |  14:38               | 6.91                             | 0.97                             | 19.87                   | 435                                 |
|    | Human      | 10.50             | 3.64             |  55:38               | 7.23                             | 1.09                             | 22.73                   | 768                                 |
|    | NCBI-virus | 10.55             | 0.54             |  04:42               | 6.63                             | 0.90                             | 14.93                   | 187                                 |
|    | SE         | 10.93             | 2.08             |  21:37               | 6.95                             | 1.00                             | 20.87                   | 290                                 |
|    | HPRC       | 10.50             | 7.78             | 180:02               | 8.07                             | 1.24                             | 25.20                   | 835                                 |

<p align="center"><b>Tab. 2 SBWT results</b></p>


FMSI indexes make use of the LCP array to speed up streaming queries.

**Note**: as of 17/11/25, the `access` query for FMSI has not been implemented yet, so it's reported as `---` in the table.

| k  | Collection | Space (bits/kmer) | Space (total GB) | Building time (m:ss) | Positive random Lookup (µs/kmer) | Negative random Lookup (µs/kmer) | Random Access (µs/kmer) | Streaming Lookup high-hit (ns/kmer) |
|----|------------|:-----------------:|:----------------:|:--------------------:|:--------------------------------:|:--------------------------------:|:-----------------------:|:-----------------------------------:|
||
| 31 | Cod        | 3.37              | 0.21             | 02:12                | 5.80                             | 1.69                             | ---                     |  369                                |
|    | Kestrel    | 3.16              | 0.45             | 05:18                | 6.22                             | 1.90                             | ---                     | 1446                                |
|    | Human      | 3.31              | 1.04             | 14:33                | 6.66                             | 2.15                             | ---                     | 1583                                |
|    | NCBI-virus | 3.50              | 0.16             | 01:39                | 5.60                             | 1.63                             | ---                     |  892                                |
|    | SE         | 4.39              | 0.49             | 05:08                | 6.57                             | 1.99                             | ---                     | 1493                                |
|    | HPRC       | 4.26              | 1.98             | 27:47                | 7.09                             | 2.31                             | ---                     | 1886                                |
||
| 63 | Cod        | 3.33              | 0.23             | 02:38                | 12.80                            | 1.77                             | ---                     |  523                                |
|    | Kestrel    | 3.17              | 0.46             | 05:41                | 13.84                            | 1.90                             | ---                     | 1833                                |
|    | Human      | 3.22              | 1.11             | 17:35                | 14.65                            | 2.15                             | ---                     | 2621                                |
|    | NCBI-virus | 3.52              | 0.18             | 02:09                | 12.38                            | 1.70                             | ---                     | 1440                                |
|    | SE         | 4.82              | 0.92             | 12:29                | 15.13                            | 2.14                             | ---                     | 1954                                |
|    | HPRC       | 4.95              | 3.67             | 61:41                | 16.05                            | 2.46                             | ---                     | 3786                                |

<p align="center"><b>Tab. 3 FMSI results</b></p>
