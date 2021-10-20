#pragma once
#include <iostream>
#include <optional>
#include <vector>

class matrix {
private:
    size_t m = 0, n = 0;
    mutable std::vector<float> data;
    mutable std::optional<size_t> selected_row = std::nullopt;

public:
    matrix() = default;
    matrix(size_t m, size_t n);

    void resize(size_t m, size_t n);

    size_t get_height() const;
    size_t get_width() const;

    matrix &raw();
    const matrix &raw() const;

    matrix &operator[](size_t i);
    const matrix &operator[](size_t i) const;

    float &operator()(size_t i, size_t j);
    const float &operator()(size_t i, size_t j) const;

    std::vector<float>::iterator begin();
    std::vector<float>::iterator end();

    std::vector<float>::const_iterator begin() const;
    std::vector<float>::const_iterator end() const;

    friend void dot(const matrix &A, const matrix &B, matrix &C, bool at, bool bt, float beta);
};

std::ostream &operator<<(std::ostream &os, const matrix &m);

std::istream &operator>>(std::istream &is, matrix &m);

void dot(const matrix &A, const matrix &B, matrix &C, bool at, bool bt, float beta);