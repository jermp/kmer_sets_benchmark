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

![](results/results-sshash.png)
<p align="center"><b>Tab. 1 SSHash results</b></p>


SBWT indexes were all built with option `--add-reverse-complements` so that queries
return the same results as for the other indexes.

![](results/results-sbwt.png)
<p align="center"><b>Tab. 2 SBWT results</b></p>


FMSI indexes make use of the LCP array to speed up streaming queries.

**Note**: as of 17/11/25, the `access` query for FMSI has not been implemented yet, so it's reported as `---` in the table.

![](results/results-fmsi.png)
<p align="center"><b>Tab. 3 FMSI results</b></p>
