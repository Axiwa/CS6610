#include <vector>

struct Vertex {
    float x;
    float y;
    float z;
};

std::vector<Vertex> vertices = {{-1,1,0}, {-1,-1,0}, {1,-1,0}, {1,1,0}};
std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};