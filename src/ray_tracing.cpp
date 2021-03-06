//
// Created by Ostap Viniavskyi on 26.04.2022.
//

#include "rays_gen.h"
#include "ray_tracing.h"


Eigen::Vector3d trace_rays(const std::vector<ray_t> &rays, const scene_t &scene, uint8_t depth) {
    if (depth == 0)
        return Vector3d{0., 0., 0.};

    const static Vector3d BACKGROUND{0.75, 0.86, 1.};
    auto color = Vector3d{0., 0., 0.};
    hit_record_t record, curr_record;
    bool has_hit;
    std::vector<ray_t> secondary_rays;

    for (const auto &ray: rays) {
        has_hit = false;
        curr_record.z = std::numeric_limits<double>::infinity();
        for (const hittable_t *object: scene) {
            if (object->intersect(ray, record)) {
                has_hit = true;
                if (record.z < curr_record.z)
                    curr_record = record;
            }
        }
        if (has_hit) {
            // Create rays using material properties
            secondary_rays = make_secondary_rays(ray, curr_record, RAYS_SPAWNED);
            Vector3d albedo = curr_record.m_ptr->get_albedo();
            color.array() += albedo.array() * trace_rays(secondary_rays, scene, depth - 1).array();
        } else {
            color += BACKGROUND;
        }
    }
    double scale = 1.0 / rays.size();
    color = (color * scale).array().sqrt();
    return color;
}

void render(image_t &image, const scene_t &scene, const camera_t &camera, int spawn_depth) {
    auto width = image.get_width(), height = image.get_height();

    for (size_t i = 0; i < width; ++i) {
        for (size_t j = 0; j < height; ++j) {
            auto rays = make_primary_rays(i, j, camera, RAYS_SPAWNED);
            auto color = trace_rays(rays, scene, spawn_depth);
            image.set_pixel(i, j, color);
        }
    }
}