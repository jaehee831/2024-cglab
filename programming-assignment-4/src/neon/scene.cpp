#include "neon/material.hpp"
#include "neon/scene.hpp"
#include "sphere.hpp"
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <random>
#include <cmath> // This includes the standard math library

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ne {

    void Scene::add(ne::RendablePointer object) {
        objects_.push_back(object);
        if (glm::length(object->material_->emitted()) > 0.0f) {
            lights_.push_back(object);
        }
    }

    bool Scene::rayIntersect(ne::Ray& ray, ne::Intersection& inter) const { 
        bool foundIntersection = false;
        for (const auto o : objects_) {
            foundIntersection = o->rayIntersect(ray, inter) || foundIntersection;
        }
        return foundIntersection;
    }

    glm::vec3 Scene::sampleBackgroundLight(const glm::vec3& dir) const {
        glm::vec3 unit = glm::normalize(dir);
        float t = 0.5f * (unit.y + 1.0f);
        return ((1.0f - t) * glm::vec3(1.0f) + t * glm::vec3(0.5, 0.5, 0.9));
    }

    glm::vec3 randomPointOnSphere(const glm::vec3& center, float radius) {
        static std::mt19937 gen{ std::random_device{}() };
        static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float u = dist(gen);
        float v = dist(gen);
        float theta = 2.0f * M_PI * u;
        float phi = acos(2.0f * v - 1.0f);
        float x = center.x + (radius * sin(phi) * cos(theta));
        float y = center.y + (radius * sin(phi) * sin(theta));
        float z = center.z + (radius * cos(phi));
        return glm::vec3(x, y, z);
    }

    glm::vec3 Scene::sampleDirectLight(ne::Ray& ray, ne::Intersection& hit) const {
        glm::vec3 lightResult(0.0f);
        const int sampleCount = 10; // Number of samples for Monte Carlo Integration

        for (auto& lightSource : lights_) {
            auto lightSphere = std::dynamic_pointer_cast<Sphere>(lightSource);
            glm::vec3 emittedColor = lightSource->material_->emitted(); // Use emitted color directly

            int currentSample = 0;
            while (currentSample < sampleCount) {
                glm::vec3 pointOnLight = randomPointOnSphere(lightSphere->center_, lightSphere->radius_);
                glm::vec3 shadowDir = glm::normalize(pointOnLight - hit.p);
                ne::Ray shadowRay(hit.p, shadowDir);

                glm::vec3 lightDirection = glm::normalize(pointOnLight - hit.p);
                glm::vec3 hitNormal = hit.n;
                float cosTheta = glm::dot(hitNormal, lightDirection);
                float distSquared = glm::dot(pointOnLight - hit.p, pointOnLight - hit.p);

                // Check for occlusion
                ne::Intersection shadowHit;
                bool isNotOccluded = !rayIntersect(shadowRay, shadowHit) || glm::distance(shadowHit.p, pointOnLight) < 0.001f;
                if (cosTheta > 0.0f && distSquared > 0.0f && isNotOccluded) {
                    lightResult += (cosTheta / distSquared) * emittedColor;
                }

                ++currentSample;
            }
        }
        lightResult /= static_cast<float>(sampleCount);
        return lightResult;
    }


} // namespace ne
