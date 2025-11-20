#include <iostream>

#include "sshash/external/pthash/external/cmd_line_parser/include/parser.hpp"

#include "fmsi/src/fmsi_api.h"
#include "fmsi/src/QSufSort.c"

#include "common.hpp"

void perf_test_lookup(fms_index& index,                    //
                      essentials::json_lines& perf_stats)  //
{
    constexpr uint64_t num_queries = 1'000'000;
    constexpr uint64_t runs = 5;
    const uint64_t k = index.k;

    std::string kmer(k, 0);
    std::string kmer_rc(k, 0);

    {
        /* perf test positive lookup */

        std::vector<std::string> lookup_queries;
        lookup_queries.reserve(num_queries);

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
            in.close();
        }

        uint64_t num_positive_kmers = 0;
        bench::timer_type t;
        t.start();
        for (uint64_t r = 0; r != runs; ++r) {
            for (auto& string : lookup_queries) {
                int64_t res = fmsi_lookup_single_query<false, true>(
                    index, string.data(), k);  // this only searches one strand
                if (res < 0) {
                    ReverseComplementStringInPlace(string.data(), k);
                    res = fmsi_lookup_single_query<false, true>(index, string.data(), k);
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
            bench::random_kmer(kmer.data(), k);
            lookup_queries.push_back(kmer);
        }

        bench::timer_type t;
        t.start();
        for (uint64_t r = 0; r != runs; ++r) {
            for (auto& string : lookup_queries) {
                int64_t res = fmsi_lookup_single_query<false, true>(index, string.data(), k);
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

        fmsi_construct_access_support(index);

        essentials::uniform_int_rng<uint64_t> distr(
            0, 10'000'000,  // this might generate a "fake" id, i.e., not corresponding to a
                            // positive kmer but we don't really care for sake of the benchmark
            essentials::get_random_seed());

        std::vector<uint64_t> access_queries;
        access_queries.reserve(num_queries);
        for (uint64_t i = 0; i != num_queries; ++i) access_queries.push_back(distr.gen());
        bench::timer_type t;
        t.start();
        for (uint64_t r = 0; r != runs; ++r) {
            for (auto id : access_queries) {
                auto kmer = fmsi_access<false>(index, id, k);
                essentials::do_not_optimize_away(kmer[0]);
            }
        }
        t.stop();
        double nanosec_per_access = t.elapsed() / static_cast<double>(runs * access_queries.size());
        std::cout << "access (avg_nanosec_per_kmer) = " << nanosec_per_access << std::endl;
        perf_stats.add("access (avg_nanosec_per_kmer)", nanosec_per_access);

        fmsi_deconstruct_access_support(index);
    }
}

void streaming_query_from_fastq_file(fms_index& index,                    //
                                     essentials::json_lines& perf_stats,  //
                                     std::istream& is)                    //
{
    essentials::timer<std::chrono::high_resolution_clock, std::chrono::microseconds> t;

    t.start();
    uint64_t total_num_kmers = 0;
    // uint64_t num_positive_kmers = 0;
    std::string line;
    const uint64_t k = index.k;
    while (!is.eof()) {
        /* We assume the file is well-formed, i.e., there are exactly 4 lines per read. */
        std::getline(is, line);  // skip first header line
        std::getline(is, line);
        if (line.size() >= k) {
            char* sequence = line.data();
            uint64_t sequence_length = line.size();
            char* rc_sequence = ReverseComplementString(sequence, sequence_length);
            std::vector<int64_t> v = fmsi_lookup_streamed_query<false, true>(
                index, sequence, rc_sequence, sequence_length, k);
            // for (auto x : v) num_positive_kmers += x >= 0;
            total_num_kmers += v.size();
            essentials::do_not_optimize_away(v[0]);
            free(rc_sequence);
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
    parser.add("index_filename", "FMSI index filename.", "-i", true);
    parser.add("query_filename", "Must be a fastq file compressed with gzip (extension fastq.gz).",
               "-q", false);
    if (!parser.parse()) return 0;
    auto index_filename = parser.get<std::string>("index_filename");

    std::string query_filename("");
    if (parser.parsed("query_filename")) {
        query_filename = parser.get<std::string>("query_filename");
    }

    essentials::logger("loading index...");
    constexpr bool use_klcp = true;
    fms_index index = load_index(index_filename, use_klcp);
    essentials::logger("DONE");

    essentials::json_lines perf_stats;

    perf_stats.add("index_filename", index_filename.c_str());
    perf_stats.add("k", index.k);

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
