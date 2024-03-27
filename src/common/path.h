#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

 

struct bezier_path {
    // Function to compute the position of a point on a 3D cubic Bezier curve and its tangent at parameter t
    static void cubicBezierCurve(const  glm::vec3* controlPoints, float t, glm::vec3& position, glm::vec3& tangent) {
        // Calculate the blending functions
        float u = 1.f - t;
        float u2 = u * u;
        float u3 = u2 * u;
        float t2 = t * t;
        float t3 = t2 * t;

        // Compute the position of the point on the Bezier curve
        position.x = u3 * controlPoints[0].x + 3.f * u2 * t * controlPoints[1].x + 3.f * u * t2 * controlPoints[2].x + t3 * controlPoints[3].x;
        position.y = u3 * controlPoints[0].y + 3.f * u2 * t * controlPoints[1].y + 3.f * u * t2 * controlPoints[2].y + t3 * controlPoints[3].y;
        position.z = u3 * controlPoints[0].z + 3.f * u2 * t * controlPoints[1].z + 3.f * u * t2 * controlPoints[2].z + t3 * controlPoints[3].z;

        // Compute the tangent at the point on the Bezier curve
        tangent.x = 3.f * (controlPoints[1].x - controlPoints[0].x) * u2 +
            6.f * (controlPoints[2].x - controlPoints[1].x) * u * t +
            3.f * (controlPoints[3].x - controlPoints[2].x) * t2;
        tangent.y = 3.f * (controlPoints[1].y - controlPoints[0].y) * u2 +
            6.f * (controlPoints[2].y - controlPoints[1].y) * u * t +
            3.f * (controlPoints[3].y - controlPoints[2].y) * t2;
        tangent.z = 3.f * (controlPoints[1].z - controlPoints[0].z) * u2 +
            6.f * (controlPoints[2].z - controlPoints[1].z) * u * t +
            3.f * (controlPoints[3].z - controlPoints[2].z) * t2;
    }

    static void regular_sampling( const std::vector<glm::vec3>& controlPoints, double delta, std::vector<glm::vec3>&samples, std::vector<glm::vec3>& tans,float * tot=0) {
  
        float eps = 0.001f;
        float residual = 0.f;
        float t = eps;
        glm::vec3 tan1;
        glm::vec3 p0 = controlPoints[0];

        
        cubicBezierCurve(&controlPoints[0], 0.f, p0, tan1);
        samples.push_back(p0);
        tans.push_back(tan1);
        float l = 0.f;
        glm::vec3 p1;
        for (unsigned int ib = 0; ib < controlPoints.size()-1; ib= ib+3) {
            const glm::vec3* bz = &controlPoints[ib];
            if (t > 1.f) {
                cubicBezierCurve(bz, t-1.f, p1, tan1);
                residual += glm::length(p1 - p0);
                p0 = p1;
                t = eps;
            }

            while (t < 1.f) {
                cubicBezierCurve(bz, t, p1, tan1);
                residual += glm::length(p1 - p0);
                t += eps;
                if (residual > delta) {
                    samples.push_back(p1);
                    tans.push_back(tan1);
                    l += residual;
                    residual = 0.f;

                }
                p0 = p1;
            }
            
        }

        if (tot)
            *tot = l;
    }

};