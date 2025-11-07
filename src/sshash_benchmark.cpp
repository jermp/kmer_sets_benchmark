#include <iostream>

#include "sshash/tools/common.hpp"
#include "sshash/include/streaming_query.hpp"

#include "sshash/src/dictionary.cpp"
#include "sshash/src/query.cpp"
#include "sshash/src/info.cpp"

using namespace sshash;

using timer_type = essentials::timer<std::chrono::high_resolution_clock, std::chrono::nanoseconds>;

void perf_test_lookup_access(dictionary_type const& index, essentials::json_lines& perf_stats)  //
{
    using kmer_t = typename dictionary_type::kmer_type;
    constexpr uint64_t num_queries = 1'000'000;
    constexpr uint64_t runs = 5;
    essentials::uniform_int_rng<uint64_t> distr(0, index.num_kmers() - 1,
                                                essentials::get_random_seed());
    const uint64_t k = index.k();
    std::string kmer(k, 0);
    std::string kmer_rc(k, 0);

    {
        // perf test positive lookup
        std::vector<std::string> lookup_queries;
        lookup_queries.reserve(num_queries);
        for (uint64_t i = 0; i != num_queries; ++i) {
            uint64_t id = distr.gen();
            index.access(id, kmer.data());
            if ((i & 1) == 0) {
                /* transform 50% of the kmers into their reverse complements */
                kmer_t::compute_reverse_complement(kmer.data(), kmer_rc.data(), k);
                lookup_queries.push_back(kmer_rc);
            } else {
                lookup_queries.push_back(kmer);
            }
        }

        uint64_t num_positive_kmers = 0;
        timer_type t;
        t.start();
        for (uint64_t r = 0; r != runs; ++r) {
            for (auto const& string : lookup_queries) {
                auto res = index.lookup(string.c_str());
                // essentials::do_not_optimize_away(res.kmer_id);
                num_positive_kmers += res.kmer_id != constants::invalid_uint64;
            }
        }
        t.stop();
        std::cout << "num_positive_kmers = " << num_positive_kmers << std::endl;
        double nanosec_per_lookup = t.elapsed() / (runs * lookup_queries.size());
        std::cout << "positive lookup (avg_nanosec_per_kmer) = " << nanosec_per_lookup << std::endl;
        perf_stats.add("positive lookup (avg_nanosec_per_kmer)", nanosec_per_lookup);
    }

    {
        // perf test negative lookup
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
                auto res = index.lookup(string.c_str());
                essentials::do_not_optimize_away(res.kmer_id);
            }
        }
        t.stop();
        double nanosec_per_lookup = t.elapsed() / (runs * lookup_queries.size());
        std::cout << "negative lookup (avg_nanosec_per_kmer) " << nanosec_per_lookup << std::endl;
        perf_stats.add("negative lookup (avg_nanosec_per_kmer)", nanosec_per_lookup);
    }

    {
        // perf test access
        std::vector<uint64_t> access_queries;
        access_queries.reserve(num_queries);
        for (uint64_t i = 0; i != num_queries; ++i) access_queries.push_back(distr.gen());
        timer_type t;
        t.start();
        for (uint64_t r = 0; r != runs; ++r) {
            for (auto id : access_queries) {
                index.access(id, kmer.data());
                essentials::do_not_optimize_away(kmer[0]);
            }
        }
        t.stop();
        double nanosec_per_access = t.elapsed() / static_cast<double>(runs * access_queries.size());
        std::cout << "access (avg_nanosec_per_kmer) = " << nanosec_per_access << std::endl;
        perf_stats.add("access (avg_nanosec_per_kmer)", nanosec_per_access);
    }
}

template <typename Query>
void streaming_query_from_fastq_file(dictionary_type const& index,        //
                                     essentials::json_lines& perf_stats,  //
                                     std::istream& is)                    //
{
    essentials::timer<std::chrono::high_resolution_clock, std::chrono::microseconds> t;

    t.start();
    uint64_t total_num_kmers = 0;
    std::string line;
    const uint64_t k = index.k();
    Query query(&index);
    while (!is.eof()) {
        query.reset();
        /* We assume the file is well-formed, i.e., there are exactly 4 lines per read. */
        std::getline(is, line);  // skip first header line
        std::getline(is, line);
        if (line.size() >= k) {
            const uint64_t num_kmers = line.size() - k + 1;
            total_num_kmers += num_kmers;
            for (uint64_t i = 0; i != num_kmers; ++i) {
                char const* kmer = line.data() + i;
                query.lookup(kmer);
            }
        }
        std::getline(is, line);  // skip '+'
        std::getline(is, line);  // skip score
    }
    t.stop();

    perf_stats.add("total_num_kmers", total_num_kmers);
    perf_stats.add("num_positive_kmers", query.num_positive_lookups());
    perf_stats.add("num_negative_kmers", query.num_negative_lookups());
    perf_stats.add("num_invalid_kmers", query.num_invalid_lookups());
    perf_stats.add("elapsed_ms", t.elapsed() / 1000);
    perf_stats.add("avg_nanosec_per_kmer", (t.elapsed() * 1000) / total_num_kmers);
}

int main(int argc, char** argv)  //
{
    cmd_line_parser::parser parser(argc, argv);
    parser.add("index_filename", "SSHash index filename.", "-i", true);
    parser.add("query_filename", "Must be a fastq file compressed with gzip (extension fastq.gz).",
               "-q", false);
    if (!parser.parse()) return 0;
    auto index_filename = parser.get<std::string>("index_filename");

    std::string query_filename("");
    if (parser.parsed("query_filename")) {
        query_filename = parser.get<std::string>("query_filename");
    }

    essentials::logger("loading index...");
    dictionary_type index;
    load_dictionary(index, index_filename, /* verbose */ false);
    essentials::logger("DONE");

    essentials::json_lines perf_stats;
    perf_stats.add("index_filename", index_filename.c_str());
    perf_stats.add("k", index.k());
    perf_stats.add("m", index.m());
    perf_stats.add("canonical", index.canonical() ? "true" : "false");

    if (query_filename.empty()) {
        perf_test_lookup_access(index, perf_stats);
    } else  //
    {
        using regular_query = streaming_query<dictionary_type, false>;
        using canonical_query = streaming_query<dictionary_type, true>;

        std::ifstream is(query_filename.c_str());
        if (!is.good()) {
            std::cerr << "error in opening the file '" + query_filename + "'" << std::endl;
            return 1;
        }
        perf_stats.add("query_filename", query_filename.c_str());
        essentials::logger("performing queries from file '" + query_filename + "'...");
        zip_istream zis(is);
        if (index.canonical()) {
            streaming_query_from_fastq_file<canonical_query>(index, perf_stats, zis);
        } else {
            streaming_query_from_fastq_file<regular_query>(index, perf_stats, zis);
        }
        is.close();
        essentials::logger("DONE");
    }

    perf_stats.print();

    return 0;
}