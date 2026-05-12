#include "bitmap/roaring_vb.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <set>
#include <stdexcept>
#include <utility>

#ifdef CUBIT_HAS_CROARING
#include <roaring/roaring.h>
#endif

namespace cubit
{

    namespace
    {
#ifdef CUBIT_HAS_CROARING
        using roaring_bitmap_ptr = roaring_bitmap_t *;

        roaring_bitmap_ptr clone_bitmap(roaring_bitmap_ptr bitmap)
        {
            return roaring_bitmap_copy(bitmap);
        }
#endif
    } // namespace

    RoaringVB::RoaringVB()
    {
#ifdef CUBIT_HAS_CROARING
        bitmap_ = roaring_bitmap_create();
#endif
    }

    RoaringVB::RoaringVB(const RoaringVB &rhs)
    {
#ifdef CUBIT_HAS_CROARING
        bitmap_ = clone_bitmap(static_cast<roaring_bitmap_ptr>(rhs.bitmap_));
#else
        values_ = rhs.values_;
#endif
    }

    RoaringVB::RoaringVB(RoaringVB &&rhs) noexcept
    {
#ifdef CUBIT_HAS_CROARING
        bitmap_ = rhs.bitmap_;
        rhs.bitmap_ = roaring_bitmap_create();
#else
        values_ = std::move(rhs.values_);
#endif
    }

    RoaringVB &RoaringVB::operator=(const RoaringVB &rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
#ifdef CUBIT_HAS_CROARING
        if (bitmap_)
        {
            roaring_bitmap_free(static_cast<roaring_bitmap_ptr>(bitmap_));
        }
        bitmap_ = clone_bitmap(static_cast<roaring_bitmap_ptr>(rhs.bitmap_));
#else
        values_ = rhs.values_;
#endif
        return *this;
    }

    RoaringVB &RoaringVB::operator=(RoaringVB &&rhs) noexcept
    {
        if (this == &rhs)
        {
            return *this;
        }
#ifdef CUBIT_HAS_CROARING
        if (bitmap_)
        {
            roaring_bitmap_free(static_cast<roaring_bitmap_ptr>(bitmap_));
        }
        bitmap_ = rhs.bitmap_;
        rhs.bitmap_ = roaring_bitmap_create();
#else
        values_ = std::move(rhs.values_);
#endif
        return *this;
    }

    RoaringVB::~RoaringVB()
    {
#ifdef CUBIT_HAS_CROARING
        if (bitmap_)
        {
            roaring_bitmap_free(static_cast<roaring_bitmap_ptr>(bitmap_));
            bitmap_ = nullptr;
        }
#else
        values_.clear();
#endif
    }

    void RoaringVB::add(uint32_t row_id)
    {
#ifdef CUBIT_HAS_CROARING
        roaring_bitmap_add(static_cast<roaring_bitmap_ptr>(bitmap_), row_id);
#else
        auto it = std::lower_bound(values_.begin(), values_.end(), row_id);
        if (it == values_.end() || *it != row_id)
        {
            values_.insert(it, row_id);
        }
#endif
    }

    void RoaringVB::remove(uint32_t row_id)
    {
#ifdef CUBIT_HAS_CROARING
        roaring_bitmap_remove(static_cast<roaring_bitmap_ptr>(bitmap_), row_id);
#else
        auto it = std::lower_bound(values_.begin(), values_.end(), row_id);
        if (it != values_.end() && *it == row_id)
        {
            values_.erase(it);
        }
#endif
    }

    bool RoaringVB::contains(uint32_t row_id) const
    {
#ifdef CUBIT_HAS_CROARING
        return roaring_bitmap_contains(static_cast<roaring_bitmap_ptr>(bitmap_), row_id);
#else
        return std::binary_search(values_.begin(), values_.end(), row_id);
#endif
    }

    RoaringVB RoaringVB::operator&(const RoaringVB &rhs) const
    {
        RoaringVB result;
#ifdef CUBIT_HAS_CROARING
        roaring_bitmap_free(static_cast<roaring_bitmap_ptr>(result.bitmap_));
        result.bitmap_ = roaring_bitmap_and(static_cast<roaring_bitmap_ptr>(bitmap_),
                                            static_cast<roaring_bitmap_ptr>(rhs.bitmap_));
#else
        std::set_intersection(values_.begin(), values_.end(), rhs.values_.begin(), rhs.values_.end(),
                              std::back_inserter(result.values_));
#endif
        return result;
    }

    RoaringVB RoaringVB::operator|(const RoaringVB &rhs) const
    {
        RoaringVB result;
#ifdef CUBIT_HAS_CROARING
        roaring_bitmap_free(static_cast<roaring_bitmap_ptr>(result.bitmap_));
        result.bitmap_ = roaring_bitmap_or(static_cast<roaring_bitmap_ptr>(bitmap_),
                                           static_cast<roaring_bitmap_ptr>(rhs.bitmap_));
#else
        std::set_union(values_.begin(), values_.end(), rhs.values_.begin(), rhs.values_.end(),
                       std::back_inserter(result.values_));
#endif
        return result;
    }

    RoaringVB &RoaringVB::operator&=(const RoaringVB &rhs)
    {
        *this = (*this & rhs);
        return *this;
    }

    RoaringVB &RoaringVB::operator|=(const RoaringVB &rhs)
    {
        *this = (*this | rhs);
        return *this;
    }

    uint64_t RoaringVB::cardinality() const
    {
#ifdef CUBIT_HAS_CROARING
        return roaring_bitmap_get_cardinality(static_cast<roaring_bitmap_ptr>(bitmap_));
#else
        return values_.size();
#endif
    }

    uint64_t RoaringVB::sizeInBytes() const
    {
#ifdef CUBIT_HAS_CROARING
        return roaring_bitmap_portable_size_in_bytes(static_cast<roaring_bitmap_ptr>(bitmap_));
#else
        return sizeof(uint32_t) * values_.capacity();
#endif
    }

    std::vector<uint32_t> RoaringVB::toVector() const
    {
#ifdef CUBIT_HAS_CROARING
        std::vector<uint32_t> values;
        values.reserve(cardinality());
        roaring_iterate(static_cast<roaring_bitmap_ptr>(bitmap_), [](uint32_t value, void *param) -> bool
                        {
                        auto *values_ptr = static_cast<std::vector<uint32_t> *>(param);
                        values_ptr->push_back(value);
                        return true; }, &values);
        return values;
#else
        return values_;
#endif
    }

    std::vector<uint8_t> RoaringVB::serialize() const
    {
#ifdef CUBIT_HAS_CROARING
        std::vector<uint8_t> bytes(sizeInBytes());
        const auto written = roaring_bitmap_portable_serialize(static_cast<roaring_bitmap_ptr>(bitmap_),
                                                               reinterpret_cast<char *>(bytes.data()));
        bytes.resize(written);
        return bytes;
#else
        std::vector<uint8_t> bytes(sizeof(uint64_t) + values_.size() * sizeof(uint32_t));
        uint64_t count = values_.size();
        std::memcpy(bytes.data(), &count, sizeof(uint64_t));
        if (!values_.empty())
        {
            std::memcpy(bytes.data() + sizeof(uint64_t), values_.data(), values_.size() * sizeof(uint32_t));
        }
        return bytes;
#endif
    }

    void RoaringVB::deserialize(const std::vector<uint8_t> &bytes)
    {
#ifdef CUBIT_HAS_CROARING
        auto *bitmap = roaring_bitmap_portable_deserialize(reinterpret_cast<const char *>(bytes.data()));
        if (!bitmap)
        {
            throw std::runtime_error("Failed to deserialize roaring bitmap");
        }
        roaring_bitmap_free(static_cast<roaring_bitmap_ptr>(bitmap_));
        bitmap_ = bitmap;
#else
        values_.clear();
        if (bytes.size() < sizeof(uint64_t))
        {
            throw std::runtime_error("Invalid bitmap payload");
        }
        uint64_t count = 0;
        std::memcpy(&count, bytes.data(), sizeof(uint64_t));
        if (bytes.size() != sizeof(uint64_t) + count * sizeof(uint32_t))
        {
            throw std::runtime_error("Invalid bitmap payload size");
        }
        values_.resize(static_cast<size_t>(count));
        if (count > 0)
        {
            std::memcpy(values_.data(), bytes.data() + sizeof(uint64_t), count * sizeof(uint32_t));
        }
#endif
    }

    void RoaringVB::clear()
    {
#ifdef CUBIT_HAS_CROARING
        if (bitmap_)
        {
            roaring_bitmap_free(static_cast<roaring_bitmap_ptr>(bitmap_));
        }
        bitmap_ = roaring_bitmap_create();
#else
        values_.clear();
#endif
    }

} // namespace cubit