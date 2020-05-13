#include "min_separation_ccd.hpp"

#include <iostream>
#include <vector>

#include <Eigen/Geometry>

#include <utils/rpoly.h>

namespace msccd {
namespace root_finder {

    bool vertexFaceMSCCD(
        const Eigen::Vector3d& vertex_start,
        const Eigen::Vector3d& face_vertex0_start,
        const Eigen::Vector3d& face_vertex1_start,
        const Eigen::Vector3d& face_vertex2_start,
        const Eigen::Vector3d& vertex_end,
        const Eigen::Vector3d& face_vertex0_end,
        const Eigen::Vector3d& face_vertex1_end,
        const Eigen::Vector3d& face_vertex2_end,
        double min_distance,
        double& toi)
    {
        const Eigen::Vector3d& x3 = vertex_start;
        const Eigen::Vector3d& x0 = face_vertex0_start;
        const Eigen::Vector3d& x1 = face_vertex1_start;
        const Eigen::Vector3d& x2 = face_vertex2_start;
        Eigen::Vector3d v3 = vertex_end - vertex_start;
        Eigen::Vector3d v0 = face_vertex0_end - face_vertex0_start;
        Eigen::Vector3d v1 = face_vertex1_end - face_vertex1_start;
        Eigen::Vector3d v2 = face_vertex2_end - face_vertex2_start;
        double h = min_distance;

        Eigen::Matrix<double, 3, 3> triX;
        triX.row(0) = x0;
        triX.row(1) = x1;
        triX.row(2) = x2;
        Eigen::Matrix<double, 4, 3> triV;
        triV.row(0) = v0;
        triV.row(1) = v1;
        triV.row(2) = v2;

        std::vector<double> ts;
        ts.reserve(30);
        double t_tmp[7];
        int count;

        count = areProximate(x0, x1, x2, x3, v0, v1, v2, v3, h, t_tmp);
        ts.insert(ts.end(), t_tmp, t_tmp + count);

        for (int i = 0; i < 3; i++) {
            count = arePointPointIntersecting(
                triX.row(i), x3, triV.row(i), v3, h, t_tmp);
            ts.insert(ts.end(), t_tmp, t_tmp + count);
        }

        for (int i = 0; i < 3; i++) {
            count = arePointLineIntersecting(
                triX.row(i), triX.row((i + 1) % 3), x3, triV.row(i),
                triV.row((i + 1) % 3), v3, h, t_tmp);
            ts.insert(ts.end(), t_tmp, t_tmp + count);
        }

        assert(ts.size() <= 30);

        bool flag = false;
        toi = 2;
        for (const double& t : ts) {
            if (t < 0.0 || t > 1.0) {
                continue;
            }

            Eigen::Vector3d p0 = x0 + t * v0;
            Eigen::Vector3d p1 = x1 + t * v1;
            Eigen::Vector3d p2 = x2 + t * v2;
            Eigen::Vector3d p3 = x3 + t * v3;

            double u, v, w; // Barycentric coordinates
            double distance = vertexFaceDist(p3, p0, p1, p2, u, v, w);

            Eigen::Vector3d relVel = v3 - (u * v0 + v * v1 + w * v2);
            Eigen::Vector3d n = (p1 - p0).cross(p2 - p0);
            double Vn = relVel.dot(n);
            // double epsilon = Vn * Vn * 1e-8;
            double epsilon = h * 1e-1;
            // double epsilon = 1e-12;

            if (abs(distance) <= h * h + epsilon) {
                if (t < toi) {
                    toi = t;
                }

                flag = true;
            }
        }

        return flag;
    }

    bool edgeEdgeMSCCD(
        const Eigen::Vector3d& edge0_vertex0_start,
        const Eigen::Vector3d& edge0_vertex1_start,
        const Eigen::Vector3d& edge1_vertex0_start,
        const Eigen::Vector3d& edge1_vertex1_start,
        const Eigen::Vector3d& edge0_vertex0_end,
        const Eigen::Vector3d& edge0_vertex1_end,
        const Eigen::Vector3d& edge1_vertex0_end,
        const Eigen::Vector3d& edge1_vertex1_end,
        double min_distance,
        double& toi)
    {
        const Eigen::Vector3d& x0 = edge0_vertex0_start;
        const Eigen::Vector3d& x1 = edge0_vertex1_start;
        const Eigen::Vector3d& x2 = edge1_vertex0_start;
        const Eigen::Vector3d& x3 = edge1_vertex1_start;
        Eigen::Vector3d v0 = edge0_vertex0_end - edge0_vertex0_start;
        Eigen::Vector3d v1 = edge0_vertex1_end - edge0_vertex1_start;
        Eigen::Vector3d v2 = edge1_vertex0_end - edge1_vertex0_start;
        Eigen::Vector3d v3 = edge1_vertex1_end - edge1_vertex1_start;
        double h = min_distance;

        bool flag = false;
        toi = 2;
        double t[7];
        int count = areEEProximate(x0, x1, x2, x3, v0, v1, v2, v3, h, t);

        assert(count <= 7);

        for (int k = 0; k < count; ++k) {
            if (t[k] < 0.0 || t[k] > 1.0) {
                continue;
            }

            Eigen::Vector3d p0 = x0 + t[k] * v0;
            Eigen::Vector3d p1 = x1 + t[k] * v1;
            Eigen::Vector3d p2 = x2 + t[k] * v2;
            Eigen::Vector3d p3 = x3 + t[k] * v3;

            double r, s;
            double distance = edgeEdgeDist(p0, p1, p2, p3, r, s);

            Eigen::Vector3d relVel
                = (v0 * (1.0 - r) + v1 * r) - (v2 * (1.0 - s) + v3 * s);
            Eigen::Vector3d n = (p1 - p0).cross(p3 - p2);

            double Vn = relVel.dot(n);
            // double epsilon = Vn * Vn * 1e-8;
            double epsilon = h * 1e-1;
            // double epsilon = 1e-12;

            if (abs(distance) <= h * h + epsilon) {
                if (t[k] < toi) {
                    toi = t[k];
                }
                flag = true;
            }
        }

        return flag;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Helper functions

    int areProximate(
        const Eigen::Vector3d& x0,
        const Eigen::Vector3d& x1,
        const Eigen::Vector3d& x2,
        const Eigen::Vector3d& x3,
        const Eigen::Vector3d& v0,
        const Eigen::Vector3d& v1,
        const Eigen::Vector3d& v2,
        const Eigen::Vector3d& v3,
        double h,
        double* dt)
    {
        Eigen::Vector3d x10 = x1 - x0;
        Eigen::Vector3d x20 = x2 - x0;
        Eigen::Vector3d x30 = x3 - x0;
        Eigen::Vector3d v10 = v1 - v0;
        Eigen::Vector3d v20 = v2 - v0;
        Eigen::Vector3d v30 = v3 - v0;

        Eigen::Vector3d p = x10.cross(x20);
        Eigen::Vector3d q = v10.cross(x20) + x10.cross(v20);
        Eigen::Vector3d r = v10.cross(v20);

        double a = x30.dot(p);
        double b = x30.dot(q) + v30.dot(p);
        double c = x30.dot(r) + v30.dot(q);
        double d = v30.dot(r);

        // Coefficients are in order of decreasing powers (coeffs[0] is t^6
        // term)
        double coeffs[7];
        coeffs[6] = a * a - p.dot(p) * h * h;
        coeffs[5] = 2.0 * a * b - (2.0 * p.dot(q)) * h * h;
        coeffs[4] = b * b + 2.0 * a * c - (q.dot(q) + 2.0 * p.dot(r)) * h * h;
        coeffs[3] = 2.0 * a * d + 2.0 * b * c - (2.0 * q.dot(r)) * h * h;
        coeffs[2] = c * c + 2.0 * b * d - r.dot(r) * h * h;
        coeffs[1] = 2.0 * c * d;
        coeffs[0] = d * d;

        int leadCoeff = 6;
        double* co = &coeffs[0];
        while (coeffs[6 - leadCoeff] == 0.0) {
            leadCoeff--;
            co++;
        }

        double poly = 1;

        // Fast polynomial checks
        if (poly > 0.001) {
            double fa[7];
            double a[7];
            int signsum = 0;
            int signzero = 0;

            for (int i = 0; i < 7; ++i) {
                a[i] = coeffs[6 - i];
                if (a[i] == 0) {
                    signzero++;
                } else {
                    if (a[i] >= 0)
                        signsum++;
                    else
                        signsum--;
                }

                fa[i] = fabs(a[i]);
            }

            // all coefficients have the same sign
            if (signsum + signzero == 7 || signsum + signzero == -7) {
                return 0;
            }

            if (fa[0] > fa[6] + fa[5] + fa[4] + fa[3] + fa[2] + fa[1]) {
                return 0;
            }

            if ((a[0] * a[1] < 0)
                && (fa[1]
                    > 2 * fa[2] + 3 * fa[3] + 4 * fa[4] + 5 * fa[5] + 6 * fa[6])
                && (a[0] * (a[0] + a[1] + a[2] + a[3])) > 0) {
                return 0;
            }

            if ((a[0] > 0) && (a[1] > 0)
                && (a[0] + a[1] > fa[6] + fa[5] + fa[4] + fa[3] + fa[2])) {
                return 0;
            }

            if ((a[0] > 0) && (a[1] > 0) && (a[2] > 0)
                && (a[0] + a[1] + a[2] > fa[6] + fa[5] + fa[4] + fa[3])) {
                return 0;
            }

            if ((a[0] > 0) && (a[1] > 0) && (a[2] > 0) && (a[3] > 0)
                && (a[0] + a[1] + a[2] + a[3] > fa[6] + fa[5] + fa[4])) {
                return 0;
            }

            if ((a[0] > 0) && (a[1] > 0) && (a[2] > 0) && (a[3] > 0)
                && (a[4] > 0)
                && (a[0] + a[1] + a[2] + a[3] + a[4] > fa[6] + fa[5])) {
                return 0;
            }

            if ((a[0] > 0) && (a[1] > 0) && (a[2] > 0) && (a[3] > 0)
                && (a[4] > 0) && (a[5] > 0)
                && (a[0] + a[1] + a[2] + a[3] + a[4] + a[5] > fa[6])) {
                return 0;
            }

            if ((a[0] < 0) && (a[1] < 0)
                && (a[0] + a[1] < -(fa[6] + fa[5] + fa[4] + fa[3] + fa[2]))) {
                return 0;
            }

            if ((a[0] < 0) && (a[1] < 0) && (a[2] < 0)
                && (a[0] + a[1] + a[2] < -(fa[6] + fa[5] + fa[4] + fa[3]))) {
                return 0;
            }

            if ((a[0] < 0) && (a[1] < 0) && (a[2] < 0) && (a[3] < 0)
                && (a[0] + a[1] + a[2] + a[3] < -(fa[6] + fa[5] + fa[4]))) {
                return 0;
            }

            if ((a[0] < 0) && (a[1] < 0) && (a[2] < 0) && (a[3] < 0)
                && (a[4] < 0)
                && (a[0] + a[1] + a[2] + a[3] + a[4] < -(fa[6] + fa[5]))) {
                return 0;
            }

            if ((a[0] < 0) && (a[1] < 0) && (a[2] < 0) && (a[3] < 0)
                && (a[4] < 0) && (a[5] < 0)
                && (a[0] + a[1] + a[2] + a[3] + a[4] + a[5] < -fa[6])) {
                return 0;
            }
        }

        // Find the roots
        RootFinder rf;
        double rl[6];
        double im[6];
        int cnt = rf.rpoly(co, leadCoeff, rl, im);
        int count = 0;
        if (cnt != -1) {
            for (size_t i = 0; i < cnt; ++i) {
                if (im[i] == 0.0) {
                    dt[count++] = rl[i];
                }
            }
        }

        return count;
    }

    int arePointPointIntersecting(
        const Eigen::Vector3d& x0,
        const Eigen::Vector3d& x1,
        const Eigen::Vector3d& v0,
        const Eigen::Vector3d& v1,
        double h,
        double* dt)
    {
        Eigen::Vector3d x10 = x1 - x0;
        Eigen::Vector3d v10 = v1 - v0;

        double coeffs[3];
        coeffs[0] = v10.dot(v10);
        coeffs[1] = 2 * v10.dot(x10);
        coeffs[2] = x10.dot(x10) - h * h;

        // check leading coeff
        int leadCoeff = 2;
        double* co = &coeffs[0];
        while (coeffs[2 - leadCoeff] == 0.0) {
            leadCoeff--;
            co++;
        }
        if (leadCoeff == -1) {
            return 0;
        }

        // Find the roots
        RootFinder rf;
        double rl[2];
        double im[2];
        int cnt = rf.rpoly(co, leadCoeff, rl, im);
        int count = 0;
        if (cnt != -1) {
            for (size_t i = 0; i < cnt; ++i) {
                if (im[i] == 0.0) {
                    dt[count++] = rl[i];
                }
            }
        }

        return count;
    }

    int arePointLineIntersecting(
        const Eigen::Vector3d& x0,
        const Eigen::Vector3d& x1,
        const Eigen::Vector3d& x2,
        const Eigen::Vector3d& v0,
        const Eigen::Vector3d& v1,
        const Eigen::Vector3d& v2,
        double h,
        double* dt)
    {
        Eigen::Vector3d x10 = x1 - x0;
        Eigen::Vector3d x20 = x2 - x0;
        Eigen::Vector3d v10 = v1 - v0;
        Eigen::Vector3d v20 = v2 - v0;

        Eigen::Vector3d p = v20.cross(v10);
        Eigen::Vector3d q = v20.cross(x10) + x20.cross(v10);
        Eigen::Vector3d r = x20.cross(x10);

        double coeffs[5];
        coeffs[0] = p.dot(p);
        coeffs[1] = 2 * p.dot(q);
        coeffs[2] = 2 * p.dot(r) + q.dot(q) - v10.dot(v10) * h * h;
        coeffs[3] = 2 * q.dot(r) - 2 * v10.dot(x10) * h * h;
        coeffs[4] = r.dot(r) - x10.dot(x10) * h * h;

        // check leading coeff
        int leadCoeff = 4;
        double* co = &coeffs[0];
        while (coeffs[4 - leadCoeff] == 0.0) {
            leadCoeff--;
            co++;
        }
        if (leadCoeff == -1) {
            return 0;
        }

        // Find the roots
        RootFinder rf;
        double rl[4];
        double im[4];
        int cnt = rf.rpoly(co, leadCoeff, rl, im);
        int count = 0;
        if (cnt != -1) {
            for (size_t i = 0; i < cnt; ++i) {
                if (im[i] == 0.0) {
                    dt[count++] = rl[i];
                }
            }
        }

        return count;
    }

    double vertexFaceDist(
        const Eigen::Vector3d& p,
        const Eigen::Vector3d& a,
        const Eigen::Vector3d& b,
        const Eigen::Vector3d& c,
        double& t1,
        double& t2,
        double& t3)
    {
        double ab[3], ac[3], ap[3], bp[3];

        ab[0] = b[0] - a[0];
        ab[1] = b[1] - a[1];
        ab[2] = b[2] - a[2];

        ac[0] = c[0] - a[0];
        ac[1] = c[1] - a[1];
        ac[2] = c[2] - a[2];

        ap[0] = p[0] - a[0];
        ap[1] = p[1] - a[1];
        ap[2] = p[2] - a[2];

        double d1 = ab[0] * ap[0] + ab[1] * ap[1] + ab[2] * ap[2];
        double d2 = ac[0] * ap[0] + ac[1] * ap[1] + ac[2] * ap[2];

        if ((d1 <= 0.0f) && (d2 <= 0.0f)) {
            t1 = 1.0f;
            t2 = 0.0f;
            t3 = 0.0f;

            return (
                (p[0] - a[0]) * (p[0] - a[0]) + (p[1] - a[1]) * (p[1] - a[1])
                + (p[2] - a[2]) * (p[2] - a[2]));
        }

        bp[0] = p[0] - b[0];
        bp[1] = p[1] - b[1];
        bp[2] = p[2] - b[2];

        double d3 = ab[0] * bp[0] + ab[1] * bp[1] + ab[2] * bp[2];
        double d4 = ac[0] * bp[0] + ac[1] * bp[1] + ac[2] * bp[2];

        if ((d3 >= 0.0f) && (d4 <= d3)) {
            t1 = 0.0f;
            t2 = 1.0f;
            t3 = 0.0f;

            return (
                (p[0] - b[0]) * (p[0] - b[0]) + (p[1] - b[1]) * (p[1] - b[1])
                + (p[2] - b[2]) * (p[2] - b[2]));
        }

        double vc = d1 * d4 - d3 * d2;

        if ((vc <= 0.0f) && (d1 >= 0.0f) && (d3 <= 0.0f)) {
            double v = d1 / (d1 - d3);

            t1 = 1 - v;
            t2 = v;
            t3 = 0;

            double vec[3];
            vec[0] = p[0] - (a[0] + v * ab[0]);
            vec[1] = p[1] - (a[1] + v * ab[1]);
            vec[2] = p[2] - (a[2] + v * ab[2]);

            return (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
        }

        double cp[3];
        cp[0] = p[0] - c[0];
        cp[1] = p[1] - c[1];
        cp[2] = p[2] - c[2];

        double d5 = ab[0] * cp[0] + ab[1] * cp[1] + ab[2] * cp[2];
        double d6 = ac[0] * cp[0] + ac[1] * cp[1] + ac[2] * cp[2];

        if ((d6 >= 0.0f) && (d5 <= d6)) {
            t1 = 0;
            t2 = 0;
            t3 = 1;

            return (
                (p[0] - c[0]) * (p[0] - c[0]) + (p[1] - c[1]) * (p[1] - c[1])
                + (p[2] - c[2]) * (p[2] - c[2]));
        }

        double vb = d5 * d2 - d1 * d6;

        if ((vb <= 0.0f) && (d2 >= 0.0f) && (d6 <= 0.0f)) {
            double w = d2 / (d2 - d6);

            t1 = 1 - w;
            t2 = 0;
            t3 = w;

            double vec[3];
            vec[0] = p[0] - (a[0] + w * ac[0]);
            vec[1] = p[1] - (a[1] + w * ac[1]);
            vec[2] = p[2] - (a[2] + w * ac[2]);

            return (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
        }

        double va = d3 * d6 - d5 * d4;

        if ((va <= 0.0f) && ((d4 - d3) >= 0.0f) && ((d5 - d6) >= 0.0f)) {
            double w = (d4 - d3) / ((d4 - d3) + (d5 - d6));

            t1 = 0;
            t2 = 1 - w;
            t3 = w;

            double vec[3];
            vec[0] = p[0] - (b[0] + w * (c[0] - b[0]));
            vec[1] = p[1] - (b[1] + w * (c[1] - b[1]));
            vec[2] = p[2] - (b[2] + w * (c[2] - b[2]));

            return (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
        }

        double denom = 1.0f / (va + vb + vc);
        double v = vb * denom;
        double w = vc * denom;
        double u = 1.0 - v - w;

        t1 = u;
        t2 = v;
        t3 = w;

        double vec[3];
        vec[0] = p[0] - (u * a[0] + v * b[0] + w * c[0]);
        vec[1] = p[1] - (u * a[1] + v * b[1] + w * c[1]);
        vec[2] = p[2] - (u * a[2] + v * b[2] + w * c[2]);

        return (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    }

    int areEEProximate(
        const Eigen::Vector3d& x0,
        const Eigen::Vector3d& x1,
        const Eigen::Vector3d& x2,
        const Eigen::Vector3d& x3,
        const Eigen::Vector3d& v0,
        const Eigen::Vector3d& v1,
        const Eigen::Vector3d& v2,
        const Eigen::Vector3d& v3,
        double h,
        double* dt)
    {
        int count
            = areEdgeEdgeIntersecting(x0, x1, x2, x3, v0, v1, v2, v3, h, dt);
        return count;
    }

    double edgeEdgeDist(
        const Eigen::Vector3d& p1,
        const Eigen::Vector3d& q1,
        const Eigen::Vector3d& p2,
        const Eigen::Vector3d& q2,
        double& s,
        double& t)
    {
        double d1[3], d2[3], r[3], a, e, f;
        double c1[3], c2[3];

        d1[0] = q1[0] - p1[0];
        d1[1] = q1[1] - p1[1];
        d1[2] = q1[2] - p1[2];

        d2[0] = q2[0] - p2[0];
        d2[1] = q2[1] - p2[1];
        d2[2] = q2[2] - p2[2];

        r[0] = p1[0] - p2[0];
        r[1] = p1[1] - p2[1];
        r[2] = p1[2] - p2[2];

        a = d1[0] * d1[0] + d1[1] * d1[1] + d1[2] * d1[2];
        e = d2[0] * d2[0] + d2[1] * d2[1] + d2[2] * d2[2];
        f = d2[0] * r[0] + d2[1] * r[1] + d2[2] * r[2];

        // check if either or both segments degenerate into points
        if ((a <= 1e-8) && (e <= 1e-8)) {
            s = t = 0.0f;
            c1[0] = p1[0];
            c1[1] = p1[1];
            c1[2] = p1[2];
            c2[0] = p2[0];
            c2[1] = p2[1];
            c2[2] = p2[2];

            return (
                (c1[0] - c2[0]) * (c1[0] - c2[0])
                + (c1[1] - c2[1]) * (c1[1] - c2[1])
                + (c1[2] - c2[2]) * (c1[2] - c2[2]));
        }

        if (a <= 1e-8) {
            // first segment degenerates into a point
            s = 0.0f;
            t = f / e;
            if (t < 0.0f)
                t = 0.0f;
            if (t > 1.0f)
                t = 1.0f;
        } else {
            double c = d1[0] * r[0] + d1[1] * r[1] + d1[2] * r[2];

            if (e <= 1e-8) {
                // second segment degenerates into a point
                t = 0.0f;
                s = -c / a;
                if (s < 0.0f)
                    s = 0.0f;
                if (s > 1.0f)
                    s = 1.0f;
            } else {
                // nondegenerate case
                double b = d1[0] * d2[0] + d1[1] * d2[1] + d1[2] * d2[2];
                double denom = a * e - b * b;

                if (denom != 0.0f) {
                    s = (b * f - c * e) / denom;
                    if (s < 0.0f)
                        s = 0.0f;
                    if (s > 1.0f)
                        s = 1.0f;
                } else
                    s = 0.0f;

                double tnom = b * s + f;
                if (tnom < 0.0f) {
                    t = 0.0f;
                    s = -c / a;
                    if (s < 0.0f)
                        s = 0.0f;
                    if (s > 1.0f)
                        s = 1.0f;
                } else if (tnom > e) {
                    t = 1.0f;
                    s = (b - c) / a;
                    if (s < 0.0f)
                        s = 0.0f;
                    if (s > 1.0f)
                        s = 1.0f;
                } else
                    t = tnom / e;
            }
        }

        c1[0] = p1[0] + d1[0] * s;
        c1[1] = p1[1] + d1[1] * s;
        c1[2] = p1[2] + d1[2] * s;

        c2[0] = p2[0] + d2[0] * t;
        c2[1] = p2[1] + d2[1] * t;
        c2[2] = p2[2] + d2[2] * t;

        return (
            (c1[0] - c2[0]) * (c1[0] - c2[0])
            + (c1[1] - c2[1]) * (c1[1] - c2[1])
            + (c1[2] - c2[2]) * (c1[2] - c2[2]));
    }

    int areEdgeEdgeIntersecting(
        const Eigen::Vector3d& x0,
        const Eigen::Vector3d& x1,
        const Eigen::Vector3d& x2,
        const Eigen::Vector3d& x3,
        const Eigen::Vector3d& v0,
        const Eigen::Vector3d& v1,
        const Eigen::Vector3d& v2,
        const Eigen::Vector3d& v3,
        double h,
        double* dt)
    {
        int count = 0;

        int count_tmp = 0;
        double dt_tmp[7];

        count_tmp = areLineLineIntersecting(
            x0, x1, x2, x3, v0, v1, v2, v3, h, dt_tmp);
        for (size_t i = 0; i < count_tmp; ++i) {
            if (dt_tmp[i] >= 0 && dt_tmp[i] <= 1) {
                dt[count] = dt_tmp[i];
                count++;
            }
        }

        return count;
    }

    int areLineLineIntersecting(
        const Eigen::Vector3d& x0,
        const Eigen::Vector3d& x1,
        const Eigen::Vector3d& x2,
        const Eigen::Vector3d& x3,
        const Eigen::Vector3d& v0,
        const Eigen::Vector3d& v1,
        const Eigen::Vector3d& v2,
        const Eigen::Vector3d& v3,
        double h,
        double* dt)
    {

        Eigen::Vector3d x10 = x1 - x0;
        Eigen::Vector3d x20 = x3 - x2;
        Eigen::Vector3d x30 = x3 - x0;
        Eigen::Vector3d v10 = v1 - v0;
        Eigen::Vector3d v20 = v3 - v2;
        Eigen::Vector3d v30 = v3 - v0;

        Eigen::Vector3d p = x10.cross(x20);
        Eigen::Vector3d q = v10.cross(x20) + x10.cross(v20);
        Eigen::Vector3d r = v10.cross(v20);

        double a = x30.dot(p);
        double b = x30.dot(q) + v30.dot(p);
        double c = x30.dot(r) + v30.dot(q);
        double d = v30.dot(r);

        // Coefficients are in order of decreasing powers (coeffs[0] is t^6
        // term)
        //
        double coeffs[7];
        coeffs[6] = ((a * a - p.dot(p) * h * h));               //
        coeffs[5] = ((2.0 * a * b - (2.0 * p.dot(q)) * h * h)); //
        coeffs[4]
            = ((b * b + 2.0 * a * c - (q.dot(q) + 2.0 * p.dot(r)) * h * h));  //
        coeffs[3] = ((2.0 * a * d + 2.0 * b * c - (2.0 * q.dot(r)) * h * h)); //
        coeffs[2] = ((c * c + 2.0 * b * d - r.dot(r) * h * h));               //
        coeffs[1] = ((2.0 * c * d));                                          //
        coeffs[0] = (d * d);                                                  //

        // check leading coeff
        //
        int leadCoeff = 6;
        double* co = &coeffs[0];
        while (coeffs[6 - leadCoeff] == 0.0) {
            leadCoeff--;
            co++;
        }
        if (leadCoeff == -1)
            return 0;

        // TODO: fast polynomial root checks

        // Find the roots
        //
        RootFinder rf;
        double rl[6];
        double im[6];
        int cnt = rf.rpoly(co, leadCoeff, rl, im);
        int count = 0;
        if (cnt != -1) {
            for (size_t i = 0; i < cnt; ++i) {
                if (im[i] == 0.0)
                    dt[count++] = rl[i];
            }
        }

        return count;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Intersection time

    int areCoplanar(
        Eigen::Vector3d& x0,
        Eigen::Vector3d& x1,
        Eigen::Vector3d& x2,
        Eigen::Vector3d& x3,
        Eigen::Vector3d& x0v,
        Eigen::Vector3d& x1v,
        Eigen::Vector3d& x2v,
        Eigen::Vector3d& x3v,
        double* dt)
    {
        Eigen::Vector3d x10 = x1 - x0;
        Eigen::Vector3d x20 = x2 - x0;
        Eigen::Vector3d x30 = x3 - x0;
        Eigen::Vector3d v10 = x1v - x0v;
        Eigen::Vector3d v20 = x2v - x0v;
        Eigen::Vector3d v30 = x3v - x0v;

        Eigen::Vector3d p = x10.cross(x20);
        Eigen::Vector3d q = v10.cross(x20) + x10.cross(v20);
        Eigen::Vector3d r = v10.cross(v20);

        double a = v30.dot(r);
        double b = x30.dot(r) + v30.dot(q);
        double c = x30.dot(q) + v30.dot(p);
        double d = x30.dot(p);

        // The polynomial coefficients
        //
        // double a = -v21[2]*v31[1]*v41[0] + v21[1]*v31[2]*v41[0] +
        // v21[2]*v31[0]*v41[1] -
        //            v21[0]*v31[2]*v41[1] - v21[1]*v31[0]*v41[2] +
        //            v21[0]*v31[1]*v41[2];
        //
        // double b = -v31[2]*v41[1]*x21[0] + v31[1]*v41[2]*x21[0] +
        // v31[2]*v41[0]*x21[1] -
        //            v31[0]*v41[2]*x21[1] - v31[1]*v41[0]*x21[2] +
        //            v31[0]*v41[1]*x21[2] + v21[2]*v41[1]*x31[0] -
        //            v21[1]*v41[2]*x31[0] - v21[2]*v41[0]*x31[1] +
        //            v21[0]*v41[2]*x31[1] + v21[1]*v41[0]*x31[2] -
        //            v21[0]*v41[1]*x31[2] - v21[2]*v31[1]*x41[0] +
        //            v21[1]*v31[2]*x41[0] + v21[2]*v31[0]*x41[1] -
        //            v21[0]*v31[2]*x41[1] - v21[1]*v31[0]*x41[2] +
        //            v21[0]*v31[1]*x41[2];
        //
        // double c = -v41[2]*x21[1]*x31[0] + v41[1]*x21[2]*x31[0] +
        // v41[2]*x21[0]*x31[1] -
        //            v41[0]*x21[2]*x31[1] - v41[1]*x21[0]*x31[2] +
        //            v41[0]*x21[1]*x31[2] + v31[2]*x21[1]*x41[0] -
        //            v31[1]*x21[2]*x41[0] - v31[2]*x21[0]*x41[1] +
        //            v31[0]*x21[2]*x41[1] + v21[2]*x31[0]*x41[1] -
        //            v21[0]*x31[2]*x41[1] + v31[1]*x21[0]*x41[2] -
        //            v31[0]*x21[1]*x41[2] - v21[1]*x31[0]*x41[2] -
        //            v21[2]*x31[1]*x41[0] + v21[1]*x31[2]*x41[0] +
        //            v21[0]*x31[1]*x41[2];
        //
        // double d = -x21[2]*x31[1]*x41[0] + x21[1]*x31[2]*x41[0] +
        // x21[2]*x31[0]*x41[1] -
        //            x21[0]*x31[2]*x41[1] - x21[1]*x31[0]*x41[2] +
        //            x21[0]*x31[1]*x41[2];

        int leadCoeff = 3;
        double coeffs[4];
        if (a == 0.0) {
            leadCoeff = 2;
            if (b == 0.0) {
                leadCoeff = 1;
                if (c == 0.0) {
                    // Degenerate polynomial
                    //
                    return 0;
                } else {
                    coeffs[0] = c;
                    coeffs[1] = d;
                }
            } else {
                coeffs[0] = b;
                coeffs[1] = c;
                coeffs[2] = d;
            }
        } else {
            coeffs[0] = a;
            coeffs[1] = b;
            coeffs[2] = c;
            coeffs[3] = d;
        }

        // Use of Sturm's theorem, not really needed for cubics...
        //
        // double seq1[4];
        // double seq2[4];

        // seq1[0] = coeffs[3];
        // seq2[0] = coeffs[0] + coeffs[1] + coeffs[2] + coeffs[3];

        // seq1[1] = coeffs[2];
        // seq2[1] = 3.0 * coeffs[0] + 2.0 * coeffs[1] + coeffs[2];
        // for (size_t i=2; i<4; ++i)
        //{
        //	double nbr1 = int(seq1[i-2] / seq1[i-1]);
        //	double nbr2 = int(seq2[i-2] / seq2[i-1]);

        //	seq1[i] = -(seq1[i-2] - (seq1[i-1] * nbr1));
        //	seq2[i] = -(seq2[i-2] - (seq2[i-1] * nbr2));
        //}

        // int nbrChanges1 = 0;
        // int nbrChanges2 = 0;
        // for (size_t i=1; i<4; ++i)
        //{
        //	if ((seq1[i-1] * seq1[i]) <= 0.0)
        //		nbrChanges1++;
        //
        //	if ((seq2[i-1] * seq2[i]) <= 0.0)
        //		nbrChanges2++;
        //}

        // if ((nbrChanges1 - nbrChanges2) == 0)
        // return 0;

        // cout << coeffs[0] << " " << coeffs[1] << " " << coeffs[2] << " " <<
        // coeffs[3] << " " << leadCoeff << endl;

        // Find the roots
        //
        // int count = cubicRoots(a, b, c, d, timeStep, dt);
        RootFinder rf;
        double rl[3];
        double im[3];
        int cnt = rf.rpoly(coeffs, leadCoeff, rl, im);
        int count = 0;
        if (cnt != -1) {
            for (size_t i = 0; i < cnt; ++i) {
                if (im[i] == 0.0)
                    dt[count++] = rl[i];
            }
        }

        return count;
    }

    bool VertexFaceIntersection(
        Eigen::Vector3d& x0,
        Eigen::Vector3d& x1,
        Eigen::Vector3d& x2,
        Eigen::Vector3d& x3,
        Eigen::Vector3d& v0,
        Eigen::Vector3d& v1,
        Eigen::Vector3d& v2,
        Eigen::Vector3d& v3,
        double& tRet)
    {
        double t[4];
        int count = areCoplanar(x0, x1, x2, x3, v0, v1, v2, v3, t);

        for (int k = 0; k < count; ++k) {
            if (t[k] > 0.0 && t[k] <= 1.0) {
                Eigen::Vector3d p0 = x0 + t[k] * v0;
                Eigen::Vector3d p1 = x1 + t[k] * v1;
                Eigen::Vector3d p2 = x2 + t[k] * v2;
                Eigen::Vector3d p3 = x3 + t[k] * v3;

                double t1, t2, t3;
                double distance = vertexFaceDist(p3, p0, p1, p2, t1, t2, t3);

                Eigen::Vector3d relVel = v3 - (t1 * v0 + t2 * v1 + t3 * v2);
                Eigen::Vector3d e1 = p1 - p0;
                Eigen::Vector3d e2 = p2 - p0;
                Eigen::Vector3d n = e1.cross(e2);

                double Vn = relVel.dot(n);
                // double epsilon = Vn * Vn * 1e-10;
                double epsilon = 1e-12;

                if (distance < epsilon) {
                    // tRet = max(t[k]-0.01, 0.0);
                    // tRet = max(t[k]-0.02, 0.0);
                    tRet = t[k];

                    return true;
                }
            }
        }

        return false;
    }

    bool EdgeEdgeIntersection(
        Eigen::Vector3d& x0,
        Eigen::Vector3d& x1,
        Eigen::Vector3d& x2,
        Eigen::Vector3d& x3,
        Eigen::Vector3d& v0,
        Eigen::Vector3d& v1,
        Eigen::Vector3d& v2,
        Eigen::Vector3d& v3,
        double& tRet)
    {
        double t[4];
        int count = areCoplanar(x0, x1, x2, x3, v0, v1, v2, v3, t);

        for (int k = 0; k < count; ++k) {
            if (t[k] > 0.0 && t[k] <= 1.0) {
                Eigen::Vector3d p0 = x0 + t[k] * v0;
                Eigen::Vector3d p1 = x1 + t[k] * v1;
                Eigen::Vector3d p2 = x2 + t[k] * v2;
                Eigen::Vector3d p3 = x3 + t[k] * v3;

                double r, s;
                double distance = edgeEdgeDist(p0, p1, p2, p3, r, s);

                Eigen::Vector3d relVel
                    = (v0 * (1.0 - r) + v1 * r) - (v2 * (1.0 - s) + v3 * s);
                Eigen::Vector3d e1 = p1 - p0;
                Eigen::Vector3d e2 = p3 - p2;
                Eigen::Vector3d n = e1.cross(e2);

                double Vn = relVel.dot(n);
                // double epsilon = Vn * Vn * 1e-10;
                double epsilon = 1e-12;

                if (distance < epsilon) {
                    // tRet = max(t[k]-0.01, 0.0);
                    // tRet = max(t[k]-0.02, 0.0);
                    tRet = t[k];

                    return true;
                }
            }
        }

        return false;
    }

    void getBarycentricCoordinates(
        Eigen::Vector3d& x0,
        Eigen::Vector3d& x1,
        Eigen::Vector3d& x2,
        Eigen::Vector3d& x3,
        bool vt,
        double w[4])
    {
        if (vt) {
            /*
            Eigen::Vector3d v0 = x1 - x0;
            Eigen::Vector3d v1 = x2 - x0;
            Eigen::Vector3d v2 = x3 - x0;

            double dot00 = v0.dot(v0);
            double dot01 = v0.dot(v1);
            double dot02 = v0.dot(v2);
            double dot11 = v1.dot(v1);
            double dot12 = v1.dot(v2);

            double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);

            w[1] = (dot11 * dot02 - dot01 * dot12) * invDenom;
            w[2] = (dot00 * dot12 - dot01 * dot02) * invDenom;

            w[0] = 1.0 - w[1] - w[2];
            w[3] = 1.0;

            if (w[0] < 0.0)
              w[0] = 0.0;

            if (w[0] > 1.0)
              w[0] = 1.0;

            if (w[1] < 0.0)
              w[1] = 0.0;

            if (w[1] > 1.0)
              w[1] = 1.0;

            if (w[2] < 0.0)
              w[2] = 0.0;

            if (w[2] > 1.0)
              w[2] = 1.0;
            */
            double ab[3], ac[3], ap[3], bp[3];

            ab[0] = x1[0] - x0[0];
            ab[1] = x1[1] - x0[1];
            ab[2] = x1[2] - x0[2];

            ac[0] = x2[0] - x0[0];
            ac[1] = x2[1] - x0[1];
            ac[2] = x2[2] - x0[2];

            ap[0] = x3[0] - x0[0];
            ap[1] = x3[1] - x0[1];
            ap[2] = x3[2] - x0[2];

            double d1 = ab[0] * ap[0] + ab[1] * ap[1] + ab[2] * ap[2];
            double d2 = ac[0] * ap[0] + ac[1] * ap[1] + ac[2] * ap[2];

            if ((d1 <= 0.0f) && (d2 <= 0.0f)) {
                w[0] = 1.0f;
                w[1] = 0.0f;
                w[2] = 0.0f;
                w[3] = 1.0f;

                return;
            }

            bp[0] = x3[0] - x1[0];
            bp[1] = x3[1] - x1[1];
            bp[2] = x3[2] - x1[2];

            double d3 = ab[0] * bp[0] + ab[1] * bp[1] + ab[2] * bp[2];
            double d4 = ac[0] * bp[0] + ac[1] * bp[1] + ac[2] * bp[2];

            if ((d3 >= 0.0f) && (d4 <= d3)) {
                w[0] = 0.0f;
                w[1] = 1.0f;
                w[2] = 0.0f;
                w[3] = 1.0f;

                return;
            }

            double vc = d1 * d4 - d3 * d2;

            if ((vc <= 0.0f) && (d1 >= 0.0f) && (d3 <= 0.0f)) {
                double v = d1 / (d1 - d3);

                w[0] = 1 - v;
                w[1] = v;
                w[2] = 0;
                w[3] = 1.0;

                return;
            }

            double cp[3];
            cp[0] = x3[0] - x2[0];
            cp[1] = x3[1] - x2[1];
            cp[2] = x3[2] - x2[2];

            double d5 = ab[0] * cp[0] + ab[1] * cp[1] + ab[2] * cp[2];
            double d6 = ac[0] * cp[0] + ac[1] * cp[1] + ac[2] * cp[2];

            if ((d6 >= 0.0f) && (d5 <= d6)) {
                w[0] = 0;
                w[1] = 0;
                w[2] = 1;
                w[3] = 1;

                return;
            }

            double vb = d5 * d2 - d1 * d6;

            if ((vb <= 0.0f) && (d2 >= 0.0f) && (d6 <= 0.0f)) {
                double v = d2 / (d2 - d6);

                w[0] = 1 - v;
                w[1] = 0;
                w[2] = v;
                w[3] = 1;

                return;
            }

            double va = d3 * d6 - d5 * d4;

            if ((va <= 0.0f) && ((d4 - d3) >= 0.0f) && ((d5 - d6) >= 0.0f)) {
                double v = (d4 - d3) / ((d4 - d3) + (d5 - d6));

                w[0] = 0;
                w[1] = 1 - v;
                w[2] = v;
                w[3] = 1.0;

                return;
            }

            double denom = 1.0f / (va + vb + vc);
            w[1] = vb * denom;
            w[2] = vc * denom;
            w[0] = 1.0 - w[1] - w[2];
            w[3] = 1.0;
        } else {
            Eigen::Vector3d v0 = x1 - x0;
            Eigen::Vector3d v1 = x3 - x2;
            Eigen::Vector3d v2 = x0 - x2;

            double dot00 = v0.dot(v0);
            double dot01 = v0.dot(v1);
            double dot02 = v0.dot(v2);
            double dot11 = v1.dot(v1);
            double dot12 = v1.dot(v2);

            // double invDenom = 1.0 / (dot00 * dot11 + dot01 * dot01);
            double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);

            w[1] = (-dot11 * dot02 + dot01 * dot12) * invDenom;
            w[3] = (dot00 * dot12 - dot01 * dot02) * invDenom;

            if (w[1] < 0.0)
                w[1] = 0.0;
            if (w[1] > 1.0)
                w[1] = 1.0;
            if (w[3] < 0.0)
                w[3] = 0.0;
            if (w[3] > 1.0)
                w[3] = 1.0;

            w[0] = 1.0 - w[1];
            w[2] = 1.0 - w[3];
            // w[1] = (dot11 * dot02 + dot01 * dot12) * invDenom;
            // w[3] = (dot00 * dot12 + dot01 * dot02) * invDenom;
            // w[0] = 1.0 - w[1];
            // w[2] = 1.0 - w[3];
        }
    }

} // namespace root_finder
} // namespace msccd
