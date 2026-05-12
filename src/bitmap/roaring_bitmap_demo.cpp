#include "bitmap/roaring_vb.h"

#include <cstdlib>
#include <iostream>
#include <vector>

using cubit::RoaringVB;

int main()
{
    RoaringVB lhs;
    RoaringVB rhs;

    for (uint32_t value : {1U, 3U, 5U, 7U, 9U, 100U, 1000U})
    {
        lhs.add(value);
    }
    for (uint32_t value : {3U, 4U, 5U, 6U, 7U, 1000U, 2000U})
    {
        rhs.add(value);
    }

    auto intersection = lhs & rhs;
    auto union_set = lhs | rhs;

    const std::vector<uint32_t> expected_intersection{3U, 5U, 7U, 1000U};
    const std::vector<uint32_t> expected_union{1U, 3U, 4U, 5U, 6U, 7U, 9U, 100U, 1000U, 2000U};

    if (intersection.toVector() != expected_intersection)
    {
        std::cerr << "intersection mismatch" << std::endl;
        return EXIT_FAILURE;
    }
    if (union_set.toVector() != expected_union)
    {
        std::cerr << "union mismatch" << std::endl;
        return EXIT_FAILURE;
    }

    auto payload = union_set.serialize();
    RoaringVB round_trip;
    round_trip.deserialize(payload);

    if (round_trip.toVector() != expected_union)
    {
        std::cerr << "serialization round-trip mismatch" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "roaring_bitmap_demo ok "
              << "cardinality=" << round_trip.cardinality() << ' '
              << "bytes=" << round_trip.sizeInBytes() << std::endl;
    return EXIT_SUCCESS;
}