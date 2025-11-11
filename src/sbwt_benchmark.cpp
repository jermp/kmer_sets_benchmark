#include <iostream>

#include "sshash/external/pthash/external/cmd_line_parser/include/parser.hpp"
#include "sshash/external/gz/zip_stream.hpp"
#include "sshash/external/gz/zip_stream.cpp"

#include "SBWT/include/sbwt/throwing_streams.hh"
#include "SBWT/include/sbwt/globals.hh"
#include "SBWT/include/sbwt/variants.hh"
#include "SBWT/include/sbwt/SubsetMatrixSelectSupport.hh"

#include "SBWT/SeqIO/include/SeqIO/SeqIO.hh"

#include "common.hpp"

using namespace sbwt;

void perf_test_lookup(plain_matrix_sbwt_t const& index,    //
                      essentials::json_lines& perf_stats)  //
{
    constexpr uint64_t num_queries = 1'000'000;
    constexpr uint64_t runs = 5;
    const uint64_t k = index.get_k();

    // essentials::uniform_int_rng<uint64_t> distr(0, index.number_of_kmers() - 1,
    //                                             essentials::get_random_seed());

    essentials::logger("building select support for `get_kmer_fast`...");
    SubsetMatrixSelectSupport<sdsl::bit_vector> ss(index.get_subset_rank_structure());
    essentials::logger("DONE");

    std::string kmer(k, 0);
    std::string kmer_rc(k, 0);

    {
        /* perf test positive lookup */

        std::vector<std::string> lookup_queries;
        lookup_queries.reserve(num_queries);

        // for (uint64_t i = 0; i != num_queries; ++i) {
        //     uint64_t id = distr.gen();
        //     index.get_kmer_fast(id, kmer.data(), ss);
        //     if ((i & 1) == 0) {
        //         /* transform 50% of the kmers into their reverse complements */
        //         compute_reverse_complement(kmer.data(), kmer_rc.data(), k);
        //         lookup_queries.push_back(kmer_rc);
        //     } else {
        //         lookup_queries.push_back(kmer);
        //     }
        // }

        {
            std::ifstream in("queries.txt");
            if (!in.good()) {
                std::cout
                    << "Error: 'queries.txt' file not found! First run `./sshash_benchmark ...`"
                    << std::endl;
                return;
            }
            std::string s;
            for (uint64_t i = 0; i != num_queries and in; ++i) {
                in >> s;
                lookup_queries.push_back(s);
            }
        }

        uint64_t num_positive_kmers = 0;
        timer_type t;
        t.start();
        for (uint64_t r = 0; r != runs; ++r) {
            for (auto& string : lookup_queries) {
                int64_t res = index.search(string.c_str());  // this only searches one strand
                // essentials::do_not_optimize_away(res);
                if (res < 0) {
                    seq_io::reverse_complement_c_string(string.data(), k);
                    res = index.search(string.c_str());
                }
                num_positive_kmers += res >= 0;
            }
        }
        t.stop();
        std::cout << "num_positive_kmers = " << num_positive_kmers << std::endl;
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
                int64_t res = index.search(string.c_str());
                essentials::do_not_optimize_away(res);
            }
        }
        t.stop();
        double nanosec_per_lookup = t.elapsed() / (runs * lookup_queries.size());
        std::cout << "negative lookup (avg_nanosec_per_kmer) " << nanosec_per_lookup << std::endl;
        perf_stats.add("negative lookup (avg_nanosec_per_kmer)", nanosec_per_lookup);
    }

    {
        /* perf test access */

        essentials::uniform_int_rng<uint64_t> distr(0, index.number_of_kmers() - 1,
                                                    essentials::get_random_seed());

        std::vector<uint64_t> access_queries;
        access_queries.reserve(num_queries);
        for (uint64_t i = 0; i != num_queries; ++i) access_queries.push_back(distr.gen());
        timer_type t;
        t.start();
        for (uint64_t r = 0; r != runs; ++r) {
            for (auto id : access_queries) {
                // index.get_kmer(id, kmer.data());
                index.get_kmer_fast(id, kmer.data(), ss);
                essentials::do_not_optimize_away(kmer[0]);
            }
        }
        t.stop();
        double nanosec_per_access = t.elapsed() / static_cast<double>(runs * access_queries.size());
        std::cout << "access (avg_nanosec_per_kmer) = " << nanosec_per_access << std::endl;
        perf_stats.add("access (avg_nanosec_per_kmer)", nanosec_per_access);
    }
}

void streaming_query_from_fastq_file(plain_matrix_sbwt_t const& index,    //
                                     essentials::json_lines& perf_stats,  //
                                     std::istream& is)                    //
{
    essentials::timer<std::chrono::high_resolution_clock, std::chrono::microseconds> t;

    t.start();
    uint64_t total_num_kmers = 0;
    // uint64_t num_positive_kmers = 0;
    std::string line;
    const uint64_t k = index.get_k();
    while (!is.eof()) {
        /* We assume the file is well-formed, i.e., there are exactly 4 lines per read. */
        std::getline(is, line);  // skip first header line
        std::getline(is, line);
        if (line.size() >= k) {
            std::vector<int64_t> v = index.streaming_search(line.c_str(), line.size());
            // for (auto x : v) num_positive_kmers += x >= 0;
            total_num_kmers += v.size();
            essentials::do_not_optimize_away(v[0]);
        }
        std::getline(is, line);  // skip '+'
        std::getline(is, line);  // skip score
    }
    t.stop();

    perf_stats.add("total_num_kmers", total_num_kmers);
    // perf_stats.add("num_positive_kmers", num_positive_kmers);
    perf_stats.add("elapsed_ms", t.elapsed() / 1000);
    perf_stats.add("avg_nanosec_per_kmer", (t.elapsed() * 1000) / total_num_kmers);
}

int main(int argc, char** argv)  //
{
    cmd_line_parser::parser parser(argc, argv);
    parser.add("index_filename", "SBWT index filename. Must be the `plain-matrix` variant.", "-i",
               true);
    parser.add("query_filename", "Must be a fastq file compressed with gzip (extension fastq.gz).",
               "-q", false);
    if (!parser.parse()) return 0;
    auto index_filename = parser.get<std::string>("index_filename");

    std::string query_filename("");
    if (parser.parsed("query_filename")) {
        query_filename = parser.get<std::string>("query_filename");
    }

    plain_matrix_sbwt_t index;

    {
        essentials::logger("loading index...");
        throwing_ifstream in(index_filename, ios::binary);
        std::string variant = load_string(in.stream);  // read variant type
        if (variant != "plain-matrix") {
            std::cerr << "Error: only plain-matrix variant is supported currently" << std::endl;
            return 1;
        }
        index.load(in.stream);
        essentials::logger("DONE");
    }

    essentials::json_lines perf_stats;

    perf_stats.add("index_filename", index_filename.c_str());
    perf_stats.add("k", index.get_k());

    if (query_filename.empty()) {
        perf_test_lookup(index, perf_stats);
    } else {
        std::ifstream is(query_filename.c_str());
        if (!is.good()) {
            std::cerr << "error in opening the file '" + query_filename + "'" << std::endl;
            return 1;
        }
        perf_stats.add("query_filename", query_filename.c_str());
        essentials::logger("performing queries from file '" + query_filename + "'...");
        zip_istream zis(is);
        streaming_query_from_fastq_file(index, perf_stats, zis);
        is.close();
        essentials::logger("DONE");
    }

    perf_stats.print();

    return 0;
}
