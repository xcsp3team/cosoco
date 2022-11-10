#ifndef COSOCO_LINKEDSETITERATOR_H
#define COSOCO_LINKEDSETITERATOR_H


#include <iterator>

#include "LinkedSet.h"

namespace Cosoco {


class LinkedSetIterator : public std::iterator<std::random_access_iterator_tag, int> {
   protected:
    LinkedSet *set;
    int        current;
    int        shift;

   public:
    inline LinkedSetIterator(LinkedSet &_set, int _cur, int _shift = 1) : set(&_set), current(_cur), shift(_shift) { }


    inline LinkedSetIterator(const LinkedSetIterator &pos) : set(pos.set), current(pos.current) { }


    inline ~LinkedSetIterator() { }


   public:
    inline LinkedSetIterator &operator=(const LinkedSetIterator &other) {
        set     = other.set;
        current = other.current;
        return *this;
    }


    inline LinkedSetIterator &operator+=(int n) {
        for(int i = 0; i < n; i++) current = (shift == 1) ? set->next(current) : set->prev(current);
        return *this;
    }


    inline LinkedSetIterator &operator-=(int n) {
        for(int i = 0; i < n; i++) current = shift == 1 ? set->prev(current) : set->next(current);
        return *this;
    }


    inline LinkedSetIterator &operator++() {
        current = shift == 1 ? set->next(current) : set->prev(current);
        return *this;
    }


    inline LinkedSetIterator &operator--() {
        current = shift == 1 ? set->prev(current) : set->next(current);
        return *this;
    }


    inline LinkedSetIterator operator+(int n) {
        LinkedSetIterator pos(*this);
        for(int i = 0; i < n; i++) pos.current = (shift == 1) ? set->next(current) : set->prev(current);
        return pos;
    }


    inline LinkedSetIterator operator-(int n) {
        LinkedSetIterator pos(*this);
        for(int i = 0; i < n; i++) pos.current = shift == 1 ? set->prev(current) : set->next(current);
        return pos;
    }


    inline bool operator==(const LinkedSetIterator &other) const { return (set == other.set && current == other.current); }


    inline bool operator!=(const LinkedSetIterator &other) const { return (set != other.set || current != other.current); }


    inline bool operator<(const LinkedSetIterator &other) const {
        assert(false);
        return (false);
    }


    inline bool operator>(const LinkedSetIterator &other) const {
        assert(false);
        return (false);
    }


    inline bool operator<=(const LinkedSetIterator &other) const {
        assert(false);
        return (false);
    }


    inline bool operator>=(const LinkedSetIterator &other) const {
        assert(false);
        return (false);
    }


    inline int operator-(const LinkedSetIterator &other) {
        assert(false);
        return (false);
    }


    inline int &operator*() { return current; }


    inline int *operator->() { assert(false); return 0;}


    inline int *ptr() { assert(false); return 0;}
};
}   // namespace Cosoco
#endif   // COSOCO_DOMAINITERATOR_H
