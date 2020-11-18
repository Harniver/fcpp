// Copyright � 2020 Luigi Rapetta. All Rights Reserved.

#include <cstddef>

#include "lib/graphics/shapes.hpp"


const float fcpp::internal::Shapes::VERTEX_LINE[6] = {
    // positions        
     0.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f
};

const float fcpp::internal::Shapes::VERTEX_SQUARE[12] = {
    // positions        
     0.0f,  0.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     1.0f,  1.0f,  0.0f,
     1.0f,  0.0f,  0.0f
};

const int fcpp::internal::Shapes::INDEX_SQUARE[6] = {
    0, 1, 2,
    2, 3, 0
};

const float fcpp::internal::Shapes::VERTEX_ORTHO[36] = {
    // positions           // colors
     0.0f,  0.0f,  0.0f,   0.0f, 0.4f, 0.0f,
     0.0f,  1.0f,  0.0f,   0.0f, 1.0f, 0.0f,

     0.0f,  0.0f,  0.0f,   0.4f, 0.0f, 0.0f,
     1.0f,  0.0f,  0.0f,   1.0f, 0.0f, 0.0f,

     0.0f,  0.0f,  0.0f,   0.0f, 0.0f, 0.4f,
     0.0f,  0.0f,  1.0f,   0.0f, 0.0f, 1.0f
};

const float fcpp::internal::Shapes::VERTEX_CUBE[216] = {
    // positions           // normals         
    -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f
};

/*
const float* fcpp::internal::Shapes::getVertex(fcpp::shape sh) {
    if (sh == shape::ortho) return Shapes::VERTEX_ORTHO;
    if (sh == shape::cube) return Shapes::VERTEX_CUBE;
    return NULL;
}

const int* fcpp::internal::Shapes::getIndex(fcpp::shape sh) {
    if (sh == shape::ortho) return Shapes::INDEX_ORTHO;
    return NULL;
}
*/