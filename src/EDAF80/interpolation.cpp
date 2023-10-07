#include "interpolation.hpp"

glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
    return p0 * (1.0f - x) + p1 * x;
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
                              glm::vec3 const& p2, glm::vec3 const& p3,
                              float const t, float const x)
{
    glm::vec3 l1 = p0 * ((-t * x * x * x) + (2.0f * t * x * x) - (t * x));
    glm::vec3 l2 = p1 * (((2.0f - t) * (x * x * x)) + ((t - 3.0f) * (x * x)) + 1.0f);
    glm::vec3 l3 = p2 * (((t - 2.0f) * (x * x * x)) + ((3.0f - 2.0f * t) * (x * x)) + (t * x));
    glm::vec3 l4 = p3 * ((t * (x * x * x)) - (t * (x * x)));
    
    return l1 * l2 * l3 * l4;
}
