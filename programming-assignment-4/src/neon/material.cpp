#include "neon/material.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <random>

namespace ne {

    bool DiffuseLight::scatter(const ne::Ray& r_in, const ne::Intersection& hit,
        ne::Ray& r_out) const {
     
        return false;
    }

    glm::vec3 DiffuseLight::emitted() const {
       
        return glm::vec3(1.0f, 1.0f, 1.0f); 
    }

    glm::vec3 DiffuseLight::attenuation() const {
       
        return glm::vec3(0.0f);
    }

    bool Dielectric::scatter(const ne::Ray& r_in, const ne::Intersection& hit,
        ne::Ray& r_out) const {
        // Implement your code
        glm::vec3 outward_normal;
        glm::vec3 reflected = glm::reflect(r_in.dir, hit.n);
        glm::vec3 refracted;
        float ni_over_nt;
        float cosine;
        float reflect_prob;
       
        if (glm::dot(r_in.dir, hit.n) > 0) {
            outward_normal = -hit.n;
            ni_over_nt = IOR_;
            cosine = IOR_ * glm::dot(r_in.dir, hit.n) / glm::length(r_in.dir);
        }
        else {
            outward_normal = hit.n;
            ni_over_nt = 1.0 / IOR_;
            cosine = -glm::dot(r_in.dir, hit.n) / glm::length(r_in.dir);
        }

        if (refract(r_in.dir, outward_normal, ni_over_nt, refracted)) {
            reflect_prob = schlick(cosine, IOR_);
        }
        else {
            reflect_prob = 1.0;
        }

        if (glm::linearRand(0.0f, 1.0f) < reflect_prob) {
            r_out = ne::Ray(hit.p, reflected);
        }
        else {
            r_out = ne::Ray(hit.p, refracted);
        }

        return true;
    }

    glm::vec3 Dielectric::attenuation() const {
        // implement your code
        return glm::vec3(1.0f);
    }

    bool Dielectric::refract(const glm::vec3& v, const glm::vec3& n, float ni_over_nt, glm::vec3& refracted) {
        glm::vec3 uv = glm::normalize(v);
        float dt = glm::dot(uv, n);
        float discriminant = 1.0f - ni_over_nt * ni_over_nt * (1.0f - dt * dt);
        if (discriminant > 0) {
            refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
            return true;
        }
        else {
            return false;
        }
    }

    float Dielectric::schlick(float cosine, float ref_idx) {
        float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
        r0 = r0 * r0;
        return r0 + (1.0f - r0) * pow((1.0f - cosine), 5);
    }

    bool Lambertian::scatter(const ne::Ray& r_in, const ne::Intersection& hit,
        ne::Ray& r_out) const {
        // Implement your code
        // Calculate scatter direction
        glm::vec3 scatter_direction = hit.n + glm::sphericalRand(1.0f);

        // Check for degenerate scatter direction
        if (glm::length(scatter_direction) < 1e-8) {
            scatter_direction = hit.n;
        }

        r_out = ne::Ray(hit.p, scatter_direction);
        return true;
    }

    glm::vec3 Lambertian::attenuation() const {
        // implement your code
        return color_;
    }


    bool Metal::scatter(const ne::Ray& r_in, const ne::Intersection& hit,
        ne::Ray& r_out) const {
        // Implement your code
        // Reflect the incoming ray direction around the normal
        glm::vec3 reflected = glm::reflect(glm::normalize(r_in.dir), hit.n);

        // Add some fuzziness based on the roughness
        glm::vec3 scatter_direction = reflected + roughness_ * glm::sphericalRand(1.0f);

        // Ensure the scattered ray is still in the correct direction
        if (glm::dot(scatter_direction, hit.n) > 0) {
            r_out = ne::Ray(hit.p, scatter_direction);
            return true;
        }
        else {
            return false;
        }
    }

    glm::vec3 Metal::attenuation() const {
        // implement your code
        return color_;
    }


} // namespace ne
