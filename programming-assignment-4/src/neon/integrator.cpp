#include "integrator.hpp"
#include "neon/intersection.hpp"
#include "neon/material.hpp"
#include "neon/scene.hpp"

namespace ne {

    namespace core {

        glm::vec3 Integrator::integrate(const ne::Ray& ray,
            std::shared_ptr<ne::Scene> scene) {

            glm::vec3 accumulatedLight{ 0.0f };
            glm::vec3 colorAttenuation = glm::vec3(1.0f);
            ne::Ray activeRay = ray;

            int bounceCount = 0;
            bool intersected = true;

            while (bounceCount < 10 && intersected) {
                ne::Intersection intersection;
                intersected = scene->rayIntersect(activeRay, intersection);

                if (intersected) {
                    MaterialPointer surfaceMaterial = intersection.material;
                    ne::Ray reflectedRay;

                    if (surfaceMaterial->scatter(activeRay, intersection, reflectedRay)) {
                        glm::vec3 sampledDirectLight = scene->sampleDirectLight(reflectedRay, intersection);

                        accumulatedLight = accumulatedLight + (colorAttenuation * sampledDirectLight);

                        colorAttenuation = colorAttenuation * surfaceMaterial->attenuation();

                        activeRay = reflectedRay;
                    }

                    else {
                        accumulatedLight += colorAttenuation * surfaceMaterial->emitted();
                        break;
                    }
                }
                else {
                    accumulatedLight += colorAttenuation * scene->sampleBackgroundLight(activeRay.dir);
                }
                ++bounceCount;
            }

            return accumulatedLight;
        }

    } // namespace core
} // namespace ne


