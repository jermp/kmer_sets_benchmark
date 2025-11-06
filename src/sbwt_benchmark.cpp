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

// template <typename Dict>
// void perf_test_iterator(Dict const& dict, essentials::json_lines& perf_stats) {
//     timer_type t;
//     t.start();
//     auto it = dict.begin();
//     uint64_t n = std::min<uint64_t>(dict.num_kmers(), 100'000'000);
//     for (uint64_t i = 0; i != n; ++i) {
//         auto [kmer_id, kmer] = it.next();
//         essentials::do_not_optimize_away(kmer_id);
//         essentials::do_not_optimize_away(kmer.at(0));
//     }
//     t.stop();
//     double avg_nanosec = t.elapsed() / n;
//     std::cout << "iterator (avg_nanosec_per_kmer) = " << avg_nanosec << std::endl;
//     perf_stats.add("iterator (avg_nanosec_per_kmer)", avg_nanosec);
// }

void perf_test_lookup(plain_matrix_sbwt_t const& dict,      //
                      std::string const& queries_filename,  //
                      essentials::json_lines& perf_stats)   //
{
    constexpr uint64_t num_queries = 1'000'000;
    constexpr uint64_t runs = 5;
    const uint64_t k = dict.get_k();

    std::vector<std::string> lookup_queries;
    lookup_queries.reserve(num_queries);

    {
        /* perf test positive lookup */

        {
            // read one kmer per line
            std::ifstream in(queries_filename);
            std::string kmer;
            kmer.reserve(k);
            while (in) {
                in >> kmer;
                lookup_queries.push_back(kmer);
                if (lookup_queries.size() == num_queries) break;
            }
            in.close();
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

    lookup_queries.clear();

    {
        /* perf test negative lookup */

        {
            std::string kmer(k, 0);
            for (uint64_t i = 0; i != num_queries; ++i) {
                random_kmer(kmer.data(), k);
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
        std::cout << "negative lookup (avg_nanosec_per_kmer) " << nanosec_per_lookup << std::endl;
        perf_stats.add("negative lookup (avg_nanosec_per_kmer)", nanosec_per_lookup);
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " index.sbwt queries.fa" << std::endl;
        std::cerr << "Currently only supports the plain matrix variant and uncompressed queries"
                  << std::endl;
        return 1;
    }

    plain_matrix_sbwt_t sbwt;

    {
        std::string indexfile = argv[1];
        throwing_ifstream in(indexfile, ios::binary);
        std::string variant = load_string(in.stream);  // read variant type
        if (variant != "plain-matrix") {
            std::cerr << "Error: only plain-matrix variant is supported currently" << std::endl;
            return 1;
        }
        std::cout << "loading the index..." << std::endl;
        sbwt.load(in.stream);
        std::cout << "done" << std::endl;
    }

    std::string queries_filename = argv[2];
    essentials::json_lines perf_stats;

    perf_test_lookup(sbwt, queries_filename, perf_stats);

    perf_stats.print();

    return 0;
}
