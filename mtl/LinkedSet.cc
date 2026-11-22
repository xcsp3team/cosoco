
#include "LinkedSet.h"

#include "LinkedSetIterator.h"

using namespace Cosoco;
LinkedSetIterator           LinkedSet::begin() { return iterator(*this, _first); }
LinkedSetIterator           LinkedSet::end() { return iterator(*this, -1); }
LinkedSet::reverse_iterator LinkedSet::rbegin() { return iterator(*this, _last, -1); }
LinkedSet::reverse_iterator LinkedSet::rend() { return iterator(*this, -1, -1); }


using iterator         = LinkedSetIterator;
using reverse_iterator = LinkedSetIterator;

iterator         begin();
iterator         end();
reverse_iterator rbegin();
reverse_iterator rend();
