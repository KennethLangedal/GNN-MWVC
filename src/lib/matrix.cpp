#include "matrix.hpp"
#include <algorithm>
#include <cassert>
#include <cblas.h>
#include <numeric>

matrix::matrix(size_t m, size_t n) : m(m), n(n), data(m * n) {}

void matrix::resize(size_t m, size_t n) {
    if (this->m == m && this->n == n)
        return;
    this->m = m;
    this->n = n;
    data.resize(m * n);
    selected_row = std::nullopt;
}

size_t matrix::get_height() const { return m; }

size_t matrix::get_width() const { return n; }

matrix &matrix::raw() {
    selected_row = std::nullopt;
    return *this;
}

const matrix &matrix::raw() const {
    selected_row = std::nullopt;
    return *this;
}

matrix &matrix::operator[](size_t i) {
    assert(i < get_height());
    selected_row = i;
    return *this;
}

const matrix &matrix::operator[](size_t i) const {
    assert(i < get_height());
    selected_row = i;
    return *this;
}

float &matrix::operator()(size_t i, size_t j) {
    assert(i < get_height() && j < get_width());
    return data[(i * get_width()) + j];
}

const float &matrix::operator()(size_t i, size_t j) const {
    assert(i < get_height() && j < get_width());
    return data[(i * get_width()) + j];
}

std::vector<float>::iterator matrix::begin() {
    return std::begin(data) + (selected_row.has_value() ? get_width() * (*selected_row) : 0);
}

std::vector<float>::iterator matrix::end() {
    return std::begin(data) + (selected_row.has_value() ? get_width() * (*selected_row + 1) : data.size());
}

std::vector<float>::const_iterator matrix::begin() const {
    return std::begin(data) + (selected_row.has_value() ? get_width() * (*selected_row) : 0);
}

std::vector<float>::const_iterator matrix::end() const {
    return std::begin(data) + (selected_row.has_value() ? get_width() * (*selected_row + 1) : data.size());
}

std::ostream &operator<<(std::ostream &os, const matrix &m) {
    os << m.get_height() << " " << m.get_width() << std::endl;
    for (size_t i = 0; i < m.get_height(); i++) {
        for (auto &&w : m[i])
            os << w << " ";
        os << std::endl;
    }
    return os;
}

std::istream &operator>>(std::istream &is, matrix &m) {
    size_t h, w;
    is >> h >> w;
    m.resize(h, w);
    for (auto &&v : m.raw())
        is >> v;
    return is;
}

void dot(const matrix &A, const matrix &B, matrix &C, bool at, bool bt, float beta) {
    size_t m = at ? A.get_width() : A.get_height(),
           n = bt ? B.get_height() : B.get_width(),
           k = at ? A.get_height() : A.get_width();
    assert(k == (at ? A.get_height() : A.get_width()) && k == (bt ? B.get_width() : B.get_height()));
    C.resize(m, n);
    cblas_sgemm(
        CblasRowMajor,
        (at ? CblasTrans : CblasNoTrans),
        (bt ? CblasTrans : CblasNoTrans),
        m, n, k,
        1.0f,
        A.data.data(), A.get_width(),
        B.data.data(), B.get_width(),
        beta,
        C.data.data(), C.get_width());
}