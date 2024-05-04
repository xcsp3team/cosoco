//
// Created by audemard on 28/04/24.
//

#ifndef COSOCO_MATRIX_H
#define COSOCO_MATRIX_H

#include <memory>
#include <vector>

#include "Vec.h"
#include "XCSP3Constants.h"
namespace Cosoco {
class Matrix {
    size_t                 _nrows;
    size_t                 _ncolumns;
    size_t                 _maxrows;
    std::unique_ptr<int[]> data;

   public:
    bool starred;
    Matrix(size_t rows, size_t columns)
        : _nrows {rows}, _ncolumns {columns}, _maxrows(0), data {std::make_unique<int[]>(rows * columns)}, starred(false) { }

    size_t nrows() const { return _maxrows; }

    size_t ncolumns() const { return _ncolumns; }

    int *operator[](size_t row) { return row * _ncolumns + data.get(); }
    int &operator()(size_t row, size_t column) { return data[row * _ncolumns + column]; }
    void addTuple(vec<int> &tuple) {
        int i = 0;
        for(int idv : tuple) {
            if(idv == STAR)
                starred = true;
            (*this)(_maxrows, i++) = idv;
        }

        _maxrows++;
    }
};
}   // namespace Cosoco

#endif   // COSOCO_MATRIX_H
