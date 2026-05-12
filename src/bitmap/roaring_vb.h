#ifndef CUBIT_ROARING_VB_H_
#define CUBIT_ROARING_VB_H_

#include <cstdint>
#include <vector>

namespace cubit
{

    class RoaringVB
    {
    public:
        RoaringVB();
        RoaringVB(const RoaringVB &rhs);
        RoaringVB(RoaringVB &&rhs) noexcept;
        RoaringVB &operator=(const RoaringVB &rhs);
        RoaringVB &operator=(RoaringVB &&rhs) noexcept;
        ~RoaringVB();

        void add(uint32_t row_id);
        void remove(uint32_t row_id);
        bool contains(uint32_t row_id) const;

        RoaringVB operator&(const RoaringVB &rhs) const;
        RoaringVB operator|(const RoaringVB &rhs) const;
        RoaringVB &operator&=(const RoaringVB &rhs);
        RoaringVB &operator|=(const RoaringVB &rhs);

        uint64_t cardinality() const;
        uint64_t sizeInBytes() const;

        std::vector<uint32_t> toVector() const;
        std::vector<uint8_t> serialize() const;
        void deserialize(const std::vector<uint8_t> &bytes);

        void clear();

    private:
#ifdef CUBIT_HAS_CROARING
        void *bitmap_;
#else
        std::vector<uint32_t> values_;
#endif
    };

} // namespace cubit

#endif