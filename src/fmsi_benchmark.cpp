#include <iostream>

#include "sshash/external/pthash/external/cmd_line_parser/include/parser.hpp"
#include "sshash/external/gz/zip_stream.hpp"
#include "sshash/external/gz/zip_stream.cpp"

#include "fmsi/src/fms_index.h"
#include "fmsi/src/QSufSort.c"

#include "essentials.hpp"

using timer_type = essentials::timer<std::chrono::high_resolution_clock, std::chrono::nanoseconds>;

void random_kmer(char* kmer, uint64_t k) {
    for (uint64_t i = 0; i != k; ++i) kmer[i] = "ACGT"[rand() % 4];
}

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
            std::ifstream in("kmers.txt");
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
                int64_t res =
                    single_query_order(index, string.data(), k);  // this only searches one strand
                if (res < 0) {
                    ReverseComplementStringInPlace(string.data(), k);
                    res = single_query_order(index, string.data(), k);
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
            for (auto& string : lookup_queries) {
                int64_t res = single_query_order(index, string.data(), k);
                essentials::do_not_optimize_away(res);
            }
        }
        t.stop();
        double nanosec_per_lookup = t.elapsed() / (runs * lookup_queries.size());
        std::cout << "negative lookup (avg_nanosec_per_kmer) " << nanosec_per_lookup << std::endl;
        perf_stats.add("negative lookup (avg_nanosec_per_kmer)", nanosec_per_lookup);
    }

    // {
    //     /* perf test access */

    //     std::vector<uint64_t> access_queries;
    //     access_queries.reserve(num_queries);
    //     for (uint64_t i = 0; i != num_queries; ++i) access_queries.push_back(distr.gen());
    //     timer_type t;
    //     t.start();
    //     for (uint64_t r = 0; r != runs; ++r) {
    //         for (auto id : access_queries) {
    //             // index.get_kmer(id, kmer.data());
    //             index.get_kmer_fast(id, kmer.data(), ss);
    //             essentials::do_not_optimize_away(kmer[0]);
    //         }
    //     }
    //     t.stop();
    //     double nanosec_per_access = t.elapsed() / static_cast<double>(runs *
    //     access_queries.size()); std::cout << "access (avg_nanosec_per_kmer) = " <<
    //     nanosec_per_access << std::endl; perf_stats.add("access (avg_nanosec_per_kmer)",
    //     nanosec_per_access);
    // }
}

template <typename T>
void __swap(T& a, T& b) {
    T tmp = a;
    a = b;
    b = tmp;
}

/*
    Version of

    template <bool maximized_ones = false>
    void query_kmers_streaming(fms_index& index, char* sequence, char* rc_sequence,
        size_t sequence_length, int k, bool output_orders, std::ostream& of);

    from fmsi/fms_index.h, where maximized_ones = true, output_orders = true,
    and without IO overhead.
*/
std::vector<int64_t> query_kmers_streaming(fms_index& index, char* sequence, char* rc_sequence,
                                           size_t sequence_length, int k)  //
{
    std::vector<int64_t> result(sequence_length - k + 1, -1);
    // Use saturating counter to ensure that RC strings are visited as forward strings.
    bool should_swap = index.predictor.predict_swap();
    if (should_swap) __swap(sequence, rc_sequence);

    // Search on the forward strand.
    int forward_predictor_result = 0, backward_predictor_result = 0;
    size_t sa_start = -1, sa_end = -1;
    for (size_t i = 0; i <= sequence_length - k; ++i) {
        size_t i_back = sequence_length - k - i;
        if (sa_start == sa_end) {
            get_range_with_pattern(index, sa_start, sa_end, sequence + i_back, k);
        } else {
            extend_range_with_klcp(index, sa_start, sa_end);
            update_range(index, sa_start, sa_end, nucleotideToInt[(uint8_t)sequence[i_back]]);
        }
        result[i_back] = kmer_order_if_present(index, sa_start, sa_end);
        if (result[i_back] >= 0) {
            forward_predictor_result++;
        } else {
            forward_predictor_result--;
        }
    }

    // Search on the reverse strand.
    sa_start = sa_end = -1;
    for (size_t i = 0; i <= sequence_length - k; ++i) {
        if ((result[i] >= 0) || result[i] == 1 || (result[i] == 0)) {
            // This position can be skipped for performance.
            sa_start = sa_end = -1;
            continue;
        }
        size_t i_back = sequence_length - k - i;
        if (sa_start == sa_end) {
            get_range_with_pattern(index, sa_start, sa_end, rc_sequence + i_back, k);
        } else {
            extend_range_with_klcp(index, sa_start, sa_end);
            update_range(index, sa_start, sa_end, nucleotideToInt[(uint8_t)rc_sequence[i_back]]);
        }
        int64_t res = kmer_order_if_present(index, sa_start, sa_end);
        if (res >= 0) {
            backward_predictor_result++;
        } else {
            backward_predictor_result--;
        }
        result[i] = std::max(result[i], res);
    }

    // Log the results to the saturating counter for better future performance.
    if (should_swap) {
        std::reverse(result.begin(), result.end());
        __swap(forward_predictor_result, backward_predictor_result);
    }
    index.predictor.log_result(forward_predictor_result, backward_predictor_result);

    return result;
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
            std::vector<int64_t> v =
                query_kmers_streaming(index, sequence, rc_sequence, sequence_length, k);
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
