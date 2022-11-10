
#include "LinkedSet.h"

#include "LinkedSetIterator.h"
/// Cosoco::LinkedSetIterator Cosoco::LinkedSet::cbegin() const { return const_iterator(*this, _first); }


// Cosoco::LinkedSetIterator Cosoco::LinkedSet::cend() const { return const_iterator(*this, -1); }


Cosoco::LinkedSetIterator Cosoco::LinkedSet::begin() { return iterator(*this, _first); }


Cosoco::LinkedSetIterator Cosoco::LinkedSet::end() { return iterator(*this, -1); }


// Cosoco::LinkedSet::const_reverse_iterator Cosoco::LinkedSet::crbegin() const { return const_iterator(*this, _last,-1); }


// Cosoco::LinkedSet::const_reverse_iterator Cosoco::LinkedSet::crend() const { return const_iterator(*this, -1, -1); }


Cosoco::LinkedSet::reverse_iterator Cosoco::LinkedSet::rbegin() { return iterator(*this, _last, -1); }


Cosoco::LinkedSet::reverse_iterator Cosoco::LinkedSet::rend() { return iterator(*this, -1, -1); }
