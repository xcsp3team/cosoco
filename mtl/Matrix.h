//
// Created by audemard on 28/04/24.
//

#ifndef COSOCO_MATRIX_H
#define COSOCO_MATRIX_H

#include <memory>
#include <vector>

#include "Vec.h"
#include "utils/Constants.h"

namespace Cosoco {
template <class T>
class Matrix {
    size_t               _nrows;
    size_t               _ncolumns;
    size_t               _maxrows;
    std::unique_ptr<T[]> data;

   public:
    bool starred;
    Matrix() : data(nullptr) { }

    Matrix(size_t rows, size_t columns)
        : _nrows {rows}, _ncolumns {columns}, _maxrows(0), data {std::make_unique<T[]>(rows * columns)} { }

    void initialize(size_t rows, size_t columns) {
        _nrows    = rows;
        _ncolumns = columns;
        _maxrows  = 0;
        data      = std::make_unique<T[]>(rows * columns);
    }

    size_t nrows() const { return _maxrows; }

    size_t ncolumns() const { return _ncolumns; }

    T *operator[](size_t row) { return row * _ncolumns + data.get(); }

    T &operator()(size_t row, size_t column) { return data[row * _ncolumns + column]; }

    void addTuple(vec<T> &tuple) {
        int i = 0;
        for(int idv : tuple) {
            if(idv == STAR)
                starred = true;
            (*this)(_maxrows, i++) = idv;
        }
        _maxrows++;
    }

    void growTo(int _nr) { _maxrows = _nr; }

    void fillRow(size_t r, T value) {
        for(size_t j = 0; j < _ncolumns; j++) (*this)(r, j) = value;
    }
};
}   // namespace Cosoco

#endif   // COSOCO_MATRIX_H
