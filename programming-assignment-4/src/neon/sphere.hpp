#ifndef __SPHERE_H_
#define __SPHERE_H_

#include "neon/blueprint.hpp"
#include "neon/rendable.hpp"

namespace ne {

    class Sphere final : public ne::abstract::Rendable{
    public:
      explicit Sphere(glm::vec3 c, float r, MaterialPointer m = nullptr)
          : ne::abstract::Rendable(m), center_(c), radius_(r) {}

      bool rayIntersect(ne::Ray & ray, Intersection & hit) override;

      //glm::vec3 sample() const;  // sample 메소드 선언 추가

      glm::vec3 center_;
      float radius_;
    };

} // namespace ne
#endif // __SPHERE_H_
