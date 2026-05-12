#include "bitmap/roaring_vb.h"

#include <algorithm>
#include <cstdint>
#include <chrono>
#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <numeric>
#include <cmath>

using cubit::RoaringVB;

class Timer
{
public:
    void start() { begin = std::chrono::high_resolution_clock::now(); }

    long long elapsed_us() const
    {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    }

    double elapsed_ms() const { return elapsed_us() / 1000.0; }

private:
    std::chrono::high_resolution_clock::time_point begin;
};

// Test data generation
std::vector<uint32_t> generate_sparse_data(uint32_t max_val, uint32_t count, uint32_t seed = 42)
{
    std::mt19937 gen(seed);
    std::uniform_int_distribution<uint32_t> dis(0, max_val);
    std::vector<uint32_t> result;
    result.reserve(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        result.push_back(dis(gen));
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

std::vector<uint32_t> generate_dense_data(uint32_t max_val, uint32_t seed = 42)
{
    std::vector<uint32_t> result;
    result.reserve(max_val);
    for (uint32_t i = 0; i < max_val; ++i)
    {
        result.push_back(i);
    }
    return result;
}

std::vector<uint32_t> generate_clustered_data(uint32_t max_val, uint32_t num_clusters, uint32_t seed = 42)
{
    std::mt19937 gen(seed);
    std::vector<uint32_t> result;
    uint32_t cluster_size = max_val / num_clusters;

    for (uint32_t c = 0; c < num_clusters; ++c)
    {
        uint32_t cluster_start = c * cluster_size;
        std::uniform_int_distribution<uint32_t> dis(cluster_start, cluster_start + cluster_size - 1);
        for (uint32_t i = 0; i < cluster_size / 2; ++i)
        {
            result.push_back(dis(gen));
        }
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

// Benchmark functions
void benchmark_add_operation(const std::string &dataset_name, const std::vector<uint32_t> &data)
{
    RoaringVB vb;

    Timer timer;
    timer.start();
    for (uint32_t val : data)
    {
        vb.add(val);
    }
    long long elapsed = timer.elapsed_us();

    std::cout << "  Add " << data.size() << " values: " << elapsed << " us ("
              << (double)elapsed / data.size() << " us/op)" << std::endl;
}

void benchmark_contains_operation(const std::string &dataset_name, const std::vector<uint32_t> &data)
{
    RoaringVB vb;
    for (uint32_t val : data)
    {
        vb.add(val);
    }

    Timer timer;
    timer.start();
    uint64_t hit_count = 0;
    for (uint32_t val : data)
    {
        if (vb.contains(val))
        {
            hit_count++;
        }
    }
    long long elapsed = timer.elapsed_us();

    std::cout << "  Contains " << data.size() << " lookups: " << elapsed << " us ("
              << (double)elapsed / data.size() << " us/op, " << hit_count << " hits)" << std::endl;
}

void benchmark_and_operation(const std::vector<uint32_t> &data1, const std::vector<uint32_t> &data2)
{
    RoaringVB vb1, vb2;
    for (uint32_t val : data1)
        vb1.add(val);
    for (uint32_t val : data2)
        vb2.add(val);

    Timer timer;
    timer.start();
    auto result = vb1 & vb2;
    long long elapsed = timer.elapsed_us();

    std::cout << "  AND operation: " << elapsed << " us (result cardinality: " << result.cardinality() << ")" << std::endl;
}

void benchmark_or_operation(const std::vector<uint32_t> &data1, const std::vector<uint32_t> &data2)
{
    RoaringVB vb1, vb2;
    for (uint32_t val : data1)
        vb1.add(val);
    for (uint32_t val : data2)
        vb2.add(val);

    Timer timer;
    timer.start();
    auto result = vb1 | vb2;
    long long elapsed = timer.elapsed_us();

    std::cout << "  OR operation: " << elapsed << " us (result cardinality: " << result.cardinality() << ")" << std::endl;
}

void benchmark_serialization(const std::string &dataset_name, const std::vector<uint32_t> &data)
{
    RoaringVB vb;
    for (uint32_t val : data)
    {
        vb.add(val);
    }

    // Serialize
    Timer timer;
    timer.start();
    auto serialized = vb.serialize();
    long long serialize_time = timer.elapsed_us();

    // Deserialize
    RoaringVB vb2;
    timer.start();
    vb2.deserialize(serialized);
    long long deserialize_time = timer.elapsed_us();

    std::cout << "  Serialize " << data.size() << " bits: " << serialize_time << " us -> "
              << serialized.size() << " bytes" << std::endl;
    std::cout << "  Deserialize " << serialized.size() << " bytes: " << deserialize_time << " us" << std::endl;
}

int main()
{
    std::cout << "=== CUBIT Roaring Bitmap Benchmark (Windows) ===" << std::endl
              << std::endl;

    // Test parameters
    const uint32_t SPARSE_MAX = 1000000;
    const uint32_t SPARSE_COUNT = 10000;
    const uint32_t DENSE_MAX = 100000;
    const uint32_t DENSE_COUNT = 100000;
    const uint32_t CLUSTERED_MAX = 1000000;
    const uint32_t NUM_CLUSTERS = 100;

    // Generate datasets
    std::cout << "Generating datasets..." << std::endl;
    auto sparse1 = generate_sparse_data(SPARSE_MAX, SPARSE_COUNT, 42);
    auto sparse2 = generate_sparse_data(SPARSE_MAX, SPARSE_COUNT, 123);
    auto dense = generate_dense_data(DENSE_MAX);
    auto clustered = generate_clustered_data(CLUSTERED_MAX, NUM_CLUSTERS);
    std::cout << "  Sparse1: " << sparse1.size() << " values" << std::endl;
    std::cout << "  Sparse2: " << sparse2.size() << " values" << std::endl;
    std::cout << "  Dense: " << dense.size() << " values" << std::endl;
    std::cout << "  Clustered: " << clustered.size() << " values" << std::endl
              << std::endl;

    // Sparse data benchmarks
    std::cout << "=== Sparse Data (10K values from 1M range) ===" << std::endl;
    benchmark_add_operation("sparse", sparse1);
    benchmark_contains_operation("sparse", sparse1);
    benchmark_and_operation(sparse1, sparse2);
    benchmark_or_operation(sparse1, sparse2);
    benchmark_serialization("sparse", sparse1);
    std::cout << std::endl;

    // Dense data benchmarks
    std::cout << "=== Dense Data (100K consecutive values) ===" << std::endl;
    benchmark_add_operation("dense", dense);
    benchmark_contains_operation("dense", dense);
    benchmark_serialization("dense", dense);
    std::cout << std::endl;

    // Clustered data benchmarks
    std::cout << "=== Clustered Data (50K values in 100 clusters) ===" << std::endl;
    benchmark_add_operation("clustered", clustered);
    benchmark_contains_operation("clustered", clustered);
    benchmark_serialization("clustered", clustered);
    std::cout << std::endl;

    // Memory footprint analysis
    std::cout << "=== Memory Footprint Analysis ===" << std::endl;
    RoaringVB vb_sparse, vb_dense, vb_clustered;
    for (uint32_t val : sparse1)
        vb_sparse.add(val);
    for (uint32_t val : dense)
        vb_dense.add(val);
    for (uint32_t val : clustered)
        vb_clustered.add(val);

    std::cout << "Sparse bitmap in-memory size: " << vb_sparse.sizeInBytes() << " bytes "
              << "(" << (double)vb_sparse.sizeInBytes() / sparse1.size() << " bytes/element)" << std::endl;
    std::cout << "Dense bitmap in-memory size: " << vb_dense.sizeInBytes() << " bytes "
              << "(" << (double)vb_dense.sizeInBytes() / dense.size() << " bytes/element)" << std::endl;
    std::cout << "Clustered bitmap in-memory size: " << vb_clustered.sizeInBytes() << " bytes "
              << "(" << (double)vb_clustered.sizeInBytes() / clustered.size() << " bytes/element)" << std::endl;
    std::cout << std::endl;

    // Correctness validation
    std::cout << "=== Correctness Validation ===" << std::endl;
    RoaringVB test_vb;
    for (uint32_t i = 0; i < 1000; i += 2)
    {
        test_vb.add(i);
    }

    uint32_t errors = 0;
    for (uint32_t i = 0; i < 1000; ++i)
    {
        bool expected = (i % 2 == 0);
        bool actual = test_vb.contains(i);
        if (expected != actual)
        {
            errors++;
        }
    }

    if (errors == 0)
    {
        std::cout << "✓ Correctness check passed (1000 elements)" << std::endl;
    }
    else
    {
        std::cout << "✗ Correctness check FAILED (" << errors << " errors)" << std::endl;
    }

    // Serialization round-trip validation
    RoaringVB test_vb2;
    for (uint32_t i = 0; i < 500; ++i)
    {
        test_vb2.add(i * 2);
    }
    auto data = test_vb2.serialize();
    RoaringVB test_vb3;
    test_vb3.deserialize(data);

    if (test_vb2.cardinality() == test_vb3.cardinality())
    {
        std::cout << "✓ Serialization round-trip passed" << std::endl;
    }
    else
    {
        std::cout << "✗ Serialization round-trip FAILED" << std::endl;
    }

    std::cout << std::endl
              << "=== Benchmark Complete ===" << std::endl;
    return 0;
}
