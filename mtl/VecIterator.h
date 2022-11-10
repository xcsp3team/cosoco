#ifndef COSOCO_DOMAINITERATOR_H
#define COSOCO_DOMAINITERATOR_H


#include <iterator>

namespace Cosoco {

// Allow for(int idx : reverse(x->domain))
// Usefull for domain removal that needs to be done in reverse order:
// Sparseset ....
template <typename C>
struct reverse_wrapper {
    C &c_;
    reverse_wrapper(C &c) : c_(c) { }

    typename C::reverse_iterator begin() { return c_.rbegin(); }
    typename C::reverse_iterator end() { return c_.rend(); }
};

template <typename C, size_t N>
struct reverse_wrapper<C[N]> {
    C (&c_)[N];
    reverse_wrapper(C (&c)[N]) : c_(c) { }

    typename std::reverse_iterator<const C *> begin() { return std::rbegin(c_); }
    typename std::reverse_iterator<const C *> end() { return std::rend(c_); }
};


template <typename C>
reverse_wrapper<C> reverse(C &c) {
    return reverse_wrapper<C>(c);
}

template <typename T>
class VecIterator : public std::iterator<std::random_access_iterator_tag, T> {
    friend class VecIterator<const T>;

   protected:
    T * ptr_;
    int shift;

   public:
    inline VecIterator(T *ptr = nullptr, int _shift = 1) : ptr_(ptr), shift(_shift) { }


    inline VecIterator(const VecIterator &pos) : ptr_(pos.ptr_), shift(pos.shift) { }


    inline ~VecIterator() { }


   public:
    inline VecIterator &operator=(const VecIterator &other) {
        ptr_  = other.ptr_;
        shift = other.shift;
        return *this;
    }


    inline VecIterator &operator+=(int n) {
        ptr_ += (shift * n);
        return *this;
    }


    inline VecIterator &operator-=(int n) {
        ptr_ -= (shift * n);
        return *this;
    }


    inline VecIterator &operator++() {
        ptr_ += shift;
        return *this;
    }


    inline VecIterator &operator--() {
        ptr_ -= shift;
        return *this;
    }


    inline VecIterator operator+(int n) {
        VecIterator pos(*this);
        pos.ptr_ += (shift * n);
        return pos;
    }


    inline VecIterator operator-(int n) {
        VecIterator pos(*this);
        pos.ptr_ -= (shift * n);
        return pos;
    }


    inline bool operator==(const VecIterator &other) const { return (ptr_ == other.ptr_ && shift == other.shift); }


    inline bool operator!=(const VecIterator &other) const { return (ptr_ != other.ptr_ || shift != other.shift); }


    inline bool operator<(const VecIterator &other) const { return (ptr_ < other.ptr_ && shift == other.shift); }


    inline bool operator>(const VecIterator &other) const { return (ptr_ > other.ptr_ && shift == other.shift); }


    inline bool operator<=(const VecIterator &other) const { return (ptr_ <= other.ptr_ && shift == other.shift); }


    inline bool operator>=(const VecIterator &other) const { return (ptr_ >= other.ptr_ && shift == other.shift); }


    inline int operator-(const VecIterator &other) {
        assert(other.shift == shift);
        return (std::distance(other.ptr_, ptr_) / shift);
    }


    inline T &operator*() { return *ptr_; }


    inline T *operator->() { return ptr_; }


    inline T *ptr() { return ptr_; }
};
}   // namespace Cosoco
#endif   // COSOCO_DOMAINITERATOR_H
