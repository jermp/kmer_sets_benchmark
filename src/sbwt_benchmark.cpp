#include <iostream>

#include "throwing_streams.hh"
#include "globals.hh"
#include "variants.hh"
#include "SeqIO/SeqIO.hh"
#include "essentials.hpp"

using namespace sbwt;

using timer_type = essentials::timer<std::chrono::high_resolution_clock, std::chrono::nanoseconds>;

void random_kmer(char* kmer, uint64_t k) {
    for (uint64_t i = 0; i != k; ++i) kmer[i] = "ACGT"[rand() % 4];
}

/*
    Reverse character map:
        65    A -> T    84
        67    C -> G    71
        71    G -> C    67
        84    T -> A    65
        97    a -> t   116
        99    c -> g   103
       103    g -> c    99
       116    t -> a    97
    All other chars map to zero.
*/
constexpr char canonicalize_basepair_reverse_map[256] = {
    0,  0, 0, 0, 0, 0, 0, 0,  0, 0,  0,   0, 0,   0,  0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,
    0,  0, 0, 0, 0, 0, 0, 0,  0, 0,  0,   0, 0,   0,  0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,
    0,  0, 0, 0, 0, 0, 0, 84, 0, 71, 0,   0, 0,   67, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 65, 0, 0,
    0,  0, 0, 0, 0, 0, 0, 0,  0, 0,  116, 0, 103, 0,  0, 0, 99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,
    97, 0, 0, 0, 0, 0, 0, 0,  0, 0,  0,   0, 0,   0,  0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,
    0,  0, 0, 0, 0, 0, 0, 0,  0, 0,  0,   0, 0,   0,  0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,
    0,  0, 0, 0, 0, 0, 0, 0,  0, 0,  0,   0, 0,   0,  0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,
    0,  0, 0, 0, 0, 0, 0, 0,  0, 0,  0,   0, 0,   0,  0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,
    0,  0, 0, 0, 0, 0, 0, 0,  0, 0,  0,   0, 0,   0,  0, 0, 0,  0, 0, 0, 0, 0, 0, 0};

void compute_reverse_complement(char const* input, char* output, uint64_t size) {
    for (uint64_t i = 0; i != size; ++i) {
        int c = input[i];
        output[size - i - 1] = canonicalize_basepair_reverse_map[c];
    }
}

void perf_test_lookup(plain_matrix_sbwt_t const& dict,     //
                      essentials::json_lines& perf_stats)  //
{
    constexpr uint64_t num_queries = 1'000'000;
    constexpr uint64_t runs = 5;
    const uint64_t k = dict.get_k();

    essentials::uniform_int_rng<uint64_t> distr(0, dict.number_of_kmers() - 1,
                                                essentials::get_random_seed());

    std::string kmer(k, 0);
    std::string kmer_rc(k, 0);

    {
        /* perf test positive lookup */

        std::vector<std::string> lookup_queries;
        lookup_queries.reserve(num_queries);

        for (uint64_t i = 0; i != num_queries; ++i) {
            uint64_t id = distr.gen();
            dict.get_kmer(id, kmer.data());
            if ((i & 1) == 0) {
                /* transform 50% of the kmers into their reverse complements */
                compute_reverse_complement(kmer.data(), kmer_rc.data(), k);
                lookup_queries.push_back(kmer_rc);
            } else {
                lookup_queries.push_back(kmer);
            }
        }

        timer_type t;
        t.start();
        for (uint64_t r = 0; r != runs; ++r) {
            for (auto const& string : lookup_queries) {
                auto res = dict.search(string.c_str());
                essentials::do_not_optimize_away(res);
            }
        }
        t.stop();
        double nanosec_per_lookup = t.elapsed() / (runs * lookup_queries.size());
        std::cout << "positive lookup (avg_nanosec_per_kmer) = " << nanosec_per_lookup << std::endl;
        perf_stats.add("positive lookup (avg_nanosec_per_kmer)", nanosec_per_lookup);
    }

    {
        /* perf test negative lookup */

        std::vector<std::string> lookup_queries;
        lookup_queries.reserve(num_queries);
        for (uint64_t i = 0; i != num_queries; ++i) {
            random_kmer(kmer.data(), k);
            lookup_queries.push_back(kmer);
        }

        timer_type t;
        t.start();
        for (uint64_t r = 0; r != runs; ++r) {
            for (auto const& string : lookup_queries) {
                auto res = dict.search(string.c_str());
                essentials::do_not_optimize_away(res);
            }
        }
        t.stop();
        double nanosec_per_lookup = t.elapsed() / (runs * lookup_queries.size());
        std::cout << "negative lookup (avg_nanosec_per_kmer) " << nanosec_per_lookup << std::endl;
        perf_stats.add("negative lookup (avg_nanosec_per_kmer)", nanosec_per_lookup);
    }

    {
        // perf test access
        constexpr uint64_t num_queries_access = 100'000;
        std::vector<uint64_t> access_queries;
        access_queries.reserve(num_queries_access);
        for (uint64_t i = 0; i != num_queries_access; ++i) access_queries.push_back(distr.gen());
        timer_type t;
        t.start();
        for (uint64_t r = 0; r != runs; ++r) {
            for (auto id : access_queries) {
                dict.get_kmer(id, kmer.data());
                essentials::do_not_optimize_away(kmer[0]);
            }
        }
        t.stop();
        double nanosec_per_access = t.elapsed() / static_cast<double>(runs * access_queries.size());
        std::cout << "access (avg_nanosec_per_kmer) = " << nanosec_per_access << std::endl;
        perf_stats.add("access (avg_nanosec_per_kmer)", nanosec_per_access);
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " index.sbwt" << std::endl;
        std::cerr << "Currently only supports the plain matrix variant." << std::endl;
        return 1;
    }

    plain_matrix_sbwt_t sbwt;

    std::string index_filename = argv[1];

    {
        throwing_ifstream in(index_filename, ios::binary);
        std::string variant = load_string(in.stream);  // read variant type
        if (variant != "plain-matrix") {
            std::cerr << "Error: only plain-matrix variant is supported currently" << std::endl;
            return 1;
        }
        std::cout << "loading the index..." << std::endl;
        sbwt.load(in.stream);
        std::cout << "done" << std::endl;
    }

    essentials::json_lines perf_stats;

    perf_stats.add("index_filename", index_filename.c_str());
    perf_stats.add("k", sbwt.get_k());

    perf_test_lookup(sbwt, perf_stats);

    perf_stats.print();

    return 0;
}
