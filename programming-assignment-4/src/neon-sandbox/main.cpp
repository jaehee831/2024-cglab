#include "test.hpp"

#include "neon/camera.hpp"
#include "neon/image.hpp"
#include "neon/integrator.hpp"
#include "neon/ray.hpp"
#include "neon/scene.hpp"
#include "neon/sphere.hpp"
#include "neon/utils.hpp"

#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <memory>
#include <taskflow/taskflow.hpp>
#include <random>

int main(int argc, char* argv[]) {
    int nx = 128; //128
    int ny = 128;
   
    int spp = 128;

    // create output image
    ne::Image canvas(nx, ny);
    glm::uvec2 tilesize(32, 32);

    // Split images into set of tiles.
    // Each thread render its corresponding tile.
    std::vector<ne::TileIterator> tiles = canvas.toTiles(tilesize);

    // create scene
    std::shared_ptr<ne::Scene> scene = testScene1();


    // spwan camera
    static ne::Camera camera;
    float distToFocus = 4;
    float aperture = 0.1f;
    glm::vec3 lookfrom(0, 0, 3);
    glm::vec3 lookat(0, 0, 0);
    camera = ne::Camera(lookfrom, lookat, glm::vec3(0, 1, 0), 60,
        float(canvas.width()) / float(canvas.height()), aperture,
        distToFocus);

    // summon progress bar. this is just eye candy.
    // you can use timer class instead
    ne::utils::Progressbar progressbar(canvas.numPixels());

    // prep to build task graph
    tf::Taskflow tf;
    tf::Task taskRenderStart =
        tf.emplace([&progressbar]() { progressbar.start(); });
    tf::Task taskRenderEnd = tf.emplace([&progressbar]() { progressbar.end(); });

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    // build rendering task graph
    for (auto& tile : tiles) {
        tf::Task taskTileRender = tf.emplace([&]() {
            // Iterate pixels in tile
            for (auto& index : tile) {
                glm::vec3 color{ 0.0f };
                for (int s = 0; s < spp; ++s) {
                    float u = (float(index.x) + dis(gen)) / float(canvas.width());
                    float v = (float(index.y) + dis(gen)) / float(canvas.height());

                    // construct ray
                    ne::Ray r = camera.sample(u, v);

                    // compute color of ray sample and then add to pixel
                    ne::core::Integrator Li;
                    color += Li.integrate(r, scene);

                }

                color /= float(spp); // Average the color
                color = glm::clamp(color, 0.0f, 1.0f);

                // record to canvas
                canvas(index) = glm::u8vec4(color * 255.99f, 255.0f);

                // update progressbar and draw it every 10 progress
                if (++progressbar % 20 == 0)
                    progressbar.display();
            }
            });

        taskRenderStart.precede(taskTileRender);
        taskTileRender.precede(taskRenderEnd);
    }

    // start rendering
    tf.wait_for_all();

    canvas.save("2.png");
    return 0;
}
