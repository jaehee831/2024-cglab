#include "sphere.hpp"

namespace ne {

bool Sphere::rayIntersect(ne::Ray &ray, Intersection &hit) {
  // Geometric solution of ray-sphere intersection check
  // See scratch pixel for detail
  const float r2 = radius_ * radius_;
  glm::vec3 diff = center_ - ray.o;
  float c0 = glm::dot(diff, ray.dir);
  float d2 = glm::dot(diff, diff) - c0 * c0;
  if (d2 > r2)
    return false;
  float c1 = glm::sqrt(r2 - d2);
  float t0 = c0 - c1;
  float t1 = c0 + c1;
  if (t0 > t1)
    std::swap(t0, t1);

  if (t0 > ray.t || t1 < eps_)
    return false;

  const float t = t0 > eps_ ? t0 : t1;
  ray.t = t;
  hit.p = ray.at(t);
  hit.n = (hit.p - center_) / radius_;
  hit.material = material_;

  return true;
}


} // namespace ne
