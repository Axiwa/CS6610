#include <vector>

struct Vertex {
    float x;
    float y;
    float z;
    float s;
    float t;
};

std::vector<Vertex> vertices = {{-1,1,0,0,1}, {-1,-1,0,0,0}, {1,-1,0,1,0}, {1,1,0,1,1}};
std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};