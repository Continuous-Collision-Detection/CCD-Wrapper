// Eigen wrappers for different CCD methods
#include "ccd.hpp"

// Etienne Vouga's CCD using a root finder in floating points
#include <CTCD.h>
// Root parity method of Brochu et al. [2012]
#include <rootparitycollisiontest.h>
// Teseo's reimplementation of Brochu et al. [2012] using rationals
#include <ECCD.hpp>
// Bernstein sign classification method of Tang et al. [2014]
#include <bsc.h>
// TightCCD method of Wang et al. [2015]
#include <bsc_tightbound.h>
// Exact Minimum Separation CCD of Wang et al. [2020]
#include <CCD/ccd.hpp>
#include <doubleCCD/doubleccd.hpp>
#include <interval_ccd/interval_ccd.hpp>
#include <SAFE_CCD.h>
#include <min_separation_ccd/min_separation_ccd.hpp>

namespace ccd {

std::array<double,3> vec2double(const Eigen::Vector3d& v){
    std::array<double,3> r;
    r[0]=v[0];
    r[1]=v[1];
    r[2]=v[2];
    return r;
}
// const auto safe_format=[](const Eigen::Vector3d& v){
//     double r[3];
//     r[0]=v[0];
//     r[1]=v[1];
//     r[2]=v[2];
//     return r;
// }
// void safe_format(const Eigen::Vector3d& v, double& r[]){
//     // double r[3];
//     r[0]=v[0];
//     r[1]=v[1];
//     r[2]=v[2];
    
// }
// Detect collisions between a vertex and a triangular face.
bool vertexFaceCCD(
    const Eigen::Vector3d& vertex_start,
    const Eigen::Vector3d& face_vertex0_start,
    const Eigen::Vector3d& face_vertex1_start,
    const Eigen::Vector3d& face_vertex2_start,
    const Eigen::Vector3d& vertex_end,
    const Eigen::Vector3d& face_vertex0_end,
    const Eigen::Vector3d& face_vertex1_end,
    const Eigen::Vector3d& face_vertex2_end,
    const CCDMethod method,const double tolerance,
    const std::array<double,3> &err)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::FLOAT:
            return CTCD::vertexFaceCTCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                /*eta=*/0, toi);
        case CCDMethod::ROOT_PARITY:
            return rootparity::RootParityCollisionTest(
                       // Point at t=0
                       Vec3d(vertex_start.data()),
                       // Triangle at t = 0
                       Vec3d(face_vertex1_start.data()),
                       Vec3d(face_vertex0_start.data()),
                       Vec3d(face_vertex2_start.data()),
                       // Point at t=1
                       Vec3d(vertex_end.data()),
                       // Triangle at t = 1
                       Vec3d(face_vertex1_end.data()),
                       Vec3d(face_vertex0_end.data()),
                       Vec3d(face_vertex2_end.data()),
                       /* is_edge_edge = */ false)
                .run_test();
        case CCDMethod::RATIONAL_ROOT_PARITY:
            return eccd::vertexFaceCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end);
        case CCDMethod::BSC:
            return bsc::Intersect_VF_robust(
                // Triangle at t = 0
                Vec3d(face_vertex0_start.data()),
                Vec3d(face_vertex1_start.data()),
                Vec3d(face_vertex2_start.data()),
                // Point at t=0
                Vec3d(vertex_start.data()),
                // Triangle at t = 1
                Vec3d(face_vertex0_end.data()), Vec3d(face_vertex1_end.data()),
                Vec3d(face_vertex2_end.data()),
                // Point at t=1
                Vec3d(vertex_end.data()));
        case CCDMethod::TIGHT_CCD:
            return bsc_tightbound::Intersect_VF_robust(
                // Triangle at t = 0
                Vec3d(face_vertex0_start.data()),
                Vec3d(face_vertex1_start.data()),
                Vec3d(face_vertex2_start.data()),
                // Point at t=0
                Vec3d(vertex_start.data()),
                // Triangle at t = 1
                Vec3d(face_vertex0_end.data()), Vec3d(face_vertex1_end.data()),
                Vec3d(face_vertex2_end.data()),
                // Point at t=1
                Vec3d(vertex_end.data()));
        case CCDMethod::FLOAT_MIN_SEPARATION:
        case CCDMethod::EXACT_RATIONAL_MIN_SEPARATION:
        case CCDMethod::EXACT_DOUBLE_MIN_SEPARATION:
            return vertexFaceMSCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                /*minimum_distance=*/DEFAULT_MIN_DISTANCE, method,tolerance);
        case CCDMethod::INTERVAL:
        return vertexFaceInterval(// Edge 1 at t=0
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end);
        case CCDMethod::OURS:
        {
            throw "Trying to use OUR method, but it should not be used by this function";
            // double toi;

            // return intervalccd::vertexFaceCCD_double(
            //  vertex_start,
            //     // Triangle at t = 0
            //     face_vertex0_start, face_vertex1_start, face_vertex2_start,
            //     // Point at t=1
            //     vertex_end,
            //     // Triangle at t = 1
            //     face_vertex0_end, face_vertex1_end, face_vertex2_end,err,DEFAULT_MIN_DISTANCE,toi,tolerance);
        }
        case CCDMethod::REDON:{
            double toi;

            return intervalccd::vertexFaceCCD_Redon(
             vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,toi);
        }
        case CCDMethod::SAFE_CCD:{
            double b=safeccd::calculate_B(
                vec2double(vertex_start),vec2double(face_vertex0_start),
                vec2double(face_vertex1_start),vec2double(face_vertex2_start),
                 vec2double(vertex_end),vec2double(face_vertex0_end),
                vec2double(face_vertex1_end),vec2double(face_vertex2_end),false);
                safeccd::SAFE_CCD<double> safe;
                safe.Set_Coefficients(b);
                double t,u[3],v[3];
                double vs[3],ve[3],f0s[3],f0e[3],f1s[3],f1e[3],f2s[3],f2e[3];
                for(int i=0;i<3;i++){
                    vs[i]=vertex_start[i];ve[i]=vertex_end[i];
                    f0s[i]=face_vertex0_start[i];f0e[i]=face_vertex0_end[i];
                    f1s[i]=face_vertex1_start[i];f1e[i]=face_vertex1_end[i];
                    f2s[i]=face_vertex2_start[i];f2e[i]=face_vertex2_end[i];
                }
                return safe.Vertex_Triangle_CCD(
                    vs,ve,
                    f0s,f0e,f1s,f1e,f2s,f2e,t,u,v);
        }

        default:
            throw "Invalid CCDMethod";
        }
    } catch (const char* err) {
        // Conservative answer upon failure.
        std::cerr << "Vertex-face CCD failed because \"" << err << "\" for "
                  << method_names[method] << std::endl;
        return true;
    } catch (...) {
        // Conservative answer upon failure.
        std::cerr << "Vertex-face CCD failed for unknown reason when using "
                  << method_names[method] << std::endl;
        return true;
    }
}

// Detect collisions between two edges as they move.
bool edgeEdgeCCD(
    const Eigen::Vector3d& edge0_vertex0_start,
    const Eigen::Vector3d& edge0_vertex1_start,
    const Eigen::Vector3d& edge1_vertex0_start,
    const Eigen::Vector3d& edge1_vertex1_start,
    const Eigen::Vector3d& edge0_vertex0_end,
    const Eigen::Vector3d& edge0_vertex1_end,
    const Eigen::Vector3d& edge1_vertex0_end,
    const Eigen::Vector3d& edge1_vertex1_end,
    const CCDMethod method,const double tolerance,const std::array<double,3> &err)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::FLOAT:
            return CTCD::edgeEdgeCTCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end,
                /*eta=*/0, toi);
        case CCDMethod::ROOT_PARITY:
            return rootparity::RootParityCollisionTest(
                       // Edge 1 at t=0
                       Vec3d(edge0_vertex0_start.data()),
                       Vec3d(edge0_vertex1_start.data()),
                       // Edge 2 at t=0
                       Vec3d(edge1_vertex0_start.data()),
                       Vec3d(edge1_vertex1_start.data()),
                       // Edge 1 at t=1
                       Vec3d(edge0_vertex0_end.data()),
                       Vec3d(edge0_vertex1_end.data()),
                       // Edge 2 at t=1
                       Vec3d(edge1_vertex0_end.data()),
                       Vec3d(edge1_vertex1_end.data()),
                       /* is_edge_edge = */ true)
                .run_test();
        case CCDMethod::RATIONAL_ROOT_PARITY:
            return eccd::edgeEdgeCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end);
        case CCDMethod::BSC:
            return bsc::Intersect_EE_robust(
                // Edge 1 at t=0
                Vec3d(edge0_vertex0_start.data()),
                Vec3d(edge0_vertex1_start.data()),
                // Edge 2 at t=0
                Vec3d(edge1_vertex0_start.data()),
                Vec3d(edge1_vertex1_start.data()),
                // Edge 1 at t=1
                Vec3d(edge0_vertex0_end.data()),
                Vec3d(edge0_vertex1_end.data()),
                // Edge 2 at t=1
                Vec3d(edge1_vertex0_end.data()),
                Vec3d(edge1_vertex1_end.data()));
        case CCDMethod::TIGHT_CCD:
            return bsc_tightbound::Intersect_EE_robust(
                // Edge 1 at t=0
                Vec3d(edge0_vertex0_start.data()),
                Vec3d(edge0_vertex1_start.data()),
                // Edge 2 at t=0
                Vec3d(edge1_vertex0_start.data()),
                Vec3d(edge1_vertex1_start.data()),
                // Edge 1 at t=1
                Vec3d(edge0_vertex0_end.data()),
                Vec3d(edge0_vertex1_end.data()),
                // Edge 2 at t=1
                Vec3d(edge1_vertex0_end.data()),
                Vec3d(edge1_vertex1_end.data()));
        case CCDMethod::FLOAT_MIN_SEPARATION:
        case CCDMethod::EXACT_RATIONAL_MIN_SEPARATION:
        case CCDMethod::EXACT_DOUBLE_MIN_SEPARATION:
            return edgeEdgeMSCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end,
                /*minimum_distance=*/DEFAULT_MIN_DISTANCE, method,tolerance);
        case CCDMethod::INTERVAL:
        return edgeEdgeInterval(// Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end);
        case CCDMethod::OURS:
        {
            throw "Trying to use OUR method, but it should not be used by this function";
            // double toi;
            // return intervalccd::edgeEdgeCCD_double(
            // edge0_vertex0_start, edge0_vertex1_start,
            //     // Edge 2 at t=0
            //     edge1_vertex0_start, edge1_vertex1_start,
            //     // Edge 1 at t=1
            //     edge0_vertex0_end, edge0_vertex1_end,
            //     // Edge 2 at t=1
            //     edge1_vertex0_end, edge1_vertex1_end,err,DEFAULT_MIN_DISTANCE,toi,tolerance);
        }
        case CCDMethod::REDON:{
            double toi;
            return intervalccd::edgeEdgeCCD_Redon(
            edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end,toi);
        }
        case CCDMethod::SAFE_CCD:{
            double b=safeccd::calculate_B(
                vec2double(edge0_vertex0_start),vec2double(edge0_vertex1_start),
                vec2double(edge1_vertex0_start),vec2double(edge1_vertex1_start),
                 vec2double(edge0_vertex0_end),vec2double(edge0_vertex1_end),
                vec2double(edge1_vertex0_end),vec2double(edge1_vertex1_end),true);
                safeccd::SAFE_CCD<double> safe;
                safe.Set_Coefficients(b);
                 double t,u[3],v[3];
                double vs[3],ve[3],f0s[3],f0e[3],f1s[3],f1e[3],f2s[3],f2e[3];
                for(int i=0;i<3;i++){
                    vs[i]=edge0_vertex0_start[i];ve[i]=edge0_vertex0_end[i];
                    f0s[i]=edge0_vertex1_start[i];f0e[i]=edge0_vertex1_end[i];
                    f1s[i]=edge1_vertex0_start[i];f1e[i]=edge1_vertex0_end[i];
                    f2s[i]=edge1_vertex1_start[i];f2e[i]=edge1_vertex1_end[i];
                }
                return safe.Edge_Edge_CCD(
                    vs,ve,
                    f0s,f0e,f1s,f1e,f2s,f2e,t,u,v);
        }

        default:
            throw "Invalid CCDMethod";
        }
    } catch (const char* err) {
        // Conservative answer upon failure.
        std::cerr << "Edge-edge CCD failed because \"" << err << "\" for "
                  << method_names[method] << std::endl;
        return true;
    } catch (...) {
        // Conservative answer upon failure.
        std::cerr << "Edge-edge CCD failed for unknown reason when using "
                  << method_names[method] << std::endl;
        return true;
    }
}
bool edgeEdgeCCD_OURS(
    const Eigen::Vector3d& edge0_vertex0_start,
    const Eigen::Vector3d& edge0_vertex1_start,
    const Eigen::Vector3d& edge1_vertex0_start,
    const Eigen::Vector3d& edge1_vertex1_start,
    const Eigen::Vector3d& edge0_vertex0_end,
    const Eigen::Vector3d& edge0_vertex1_end,
    const Eigen::Vector3d& edge1_vertex0_end,
    const Eigen::Vector3d& edge1_vertex1_end,
    const std::array<double, 3>& err,
    const double ms,//TODO maybe add an assertion to check if ms is too big?
    double& toi,
    const double tolerance,
    const double pre_check_t,
    const int max_itr,
    double &output_tolerance,
    const int CCD_TYPE){
    
    return intervalccd::edgeEdgeCCD_double(
        edge0_vertex0_start,
        edge0_vertex1_start,
        edge1_vertex0_start,
        edge1_vertex1_start,
        edge0_vertex0_end,
        edge0_vertex1_end,
        edge1_vertex0_end,
        edge1_vertex1_end,err,ms,toi,tolerance,pre_check_t,max_itr,output_tolerance,CCD_TYPE);
    }
bool vertexFaceCCD_OURS(
    const Eigen::Vector3d& edge0_vertex0_start,
    const Eigen::Vector3d& edge0_vertex1_start,
    const Eigen::Vector3d& edge1_vertex0_start,
    const Eigen::Vector3d& edge1_vertex1_start,
    const Eigen::Vector3d& edge0_vertex0_end,
    const Eigen::Vector3d& edge0_vertex1_end,
    const Eigen::Vector3d& edge1_vertex0_end,
    const Eigen::Vector3d& edge1_vertex1_end,
    const std::array<double, 3>& err,
    const double ms,//TODO maybe add an assertion to check if ms is too big?
    double& toi,
    const double tolerance,
    const double pre_check_t,
    const int max_itr,
    double &output_tolerance,
    const int CCD_TYPE){
    
    
    return intervalccd::vertexFaceCCD_double(
        edge0_vertex0_start,
        edge0_vertex1_start,
        edge1_vertex0_start,
        edge1_vertex1_start,
        edge0_vertex0_end,
        edge0_vertex1_end,
        edge1_vertex0_end,
        edge1_vertex1_end,err,ms,toi,tolerance,pre_check_t,max_itr,output_tolerance,CCD_TYPE);
    }
// Detect collisions between a vertex and a triangular face.
bool vertexFaceMSCCD(
    const Eigen::Vector3d& vertex_start,
    const Eigen::Vector3d& face_vertex0_start,
    const Eigen::Vector3d& face_vertex1_start,
    const Eigen::Vector3d& face_vertex2_start,
    const Eigen::Vector3d& vertex_end,
    const Eigen::Vector3d& face_vertex0_end,
    const Eigen::Vector3d& face_vertex1_end,
    const Eigen::Vector3d& face_vertex2_end,
    const double min_distance,
    const CCDMethod method,const double tolerance,
    const std::array<double,3> &err)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::FLOAT_MIN_SEPARATION: {
            bool hit = msccd::root_finder::vertexFaceMSCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                min_distance, toi);
            if (hit && (toi < 0 || toi > 1)) {
                std::cout << toi << std::endl;
                throw "toi out of range";
            }
            return hit;
        }
        case CCDMethod::EXACT_RATIONAL_MIN_SEPARATION:
            return ccd::vertexFaceCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                min_distance);
        case CCDMethod::EXACT_DOUBLE_MIN_SEPARATION:
            return doubleccd::vertexFaceCCD(
                // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                min_distance);
        case CCDMethod::OURS:
        {
            throw "Trying to use OUR method, but it should not be used by this function";
        //     double toi;
            
        //     return intervalccd::vertexFaceCCD_double(
        // // Point at t=0
        //         vertex_start,
        //         // Triangle at t = 0
        //         face_vertex0_start, face_vertex1_start, face_vertex2_start,
        //         // Point at t=1
        //         vertex_end,
        //         // Triangle at t = 1
        //         face_vertex0_end, face_vertex1_end, face_vertex2_end,err,min_distance,toi,tolerance);
        }
        default:
            throw "Invalid Minimum Separation CCDMethod";
        }
    } catch (const char* err) {
        // Conservative answer upon failure.
        std::cerr << "Vertex-face CCD failed because \"" << err << "\" for "
                  << method_names[method] << std::endl;
        return true;
    } catch (...) {
        // Conservative answer upon failure.
        std::cerr << "Vertex-face CCD failed for unknown reason when using "
                  << method_names[method] << std::endl;
        return true;
    }
}

// Detect collisions between two edges as they move.
bool edgeEdgeMSCCD(
    const Eigen::Vector3d& edge0_vertex0_start,
    const Eigen::Vector3d& edge0_vertex1_start,
    const Eigen::Vector3d& edge1_vertex0_start,
    const Eigen::Vector3d& edge1_vertex1_start,
    const Eigen::Vector3d& edge0_vertex0_end,
    const Eigen::Vector3d& edge0_vertex1_end,
    const Eigen::Vector3d& edge1_vertex0_end,
    const Eigen::Vector3d& edge1_vertex1_end,
    const double min_distance,
    const CCDMethod method,const double tolerance,
    const std::array<double,3> &err)
{
    double toi; // Computed by some methods but never returned
    try {
        switch (method) {
        case CCDMethod::FLOAT_MIN_SEPARATION: {
            bool hit = msccd::root_finder::edgeEdgeMSCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end, min_distance, toi);
            if (hit && (toi < 0 || toi > 1)) {
                std::cout << toi << std::endl;
                throw "toi out of range";
            }
            return hit;
        }
        case CCDMethod::EXACT_RATIONAL_MIN_SEPARATION:
            return ccd::edgeEdgeCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end, min_distance);
        case CCDMethod::EXACT_DOUBLE_MIN_SEPARATION:
            return doubleccd::edgeEdgeCCD(
                // Edge 1 at t=0
                edge0_vertex0_start, edge0_vertex1_start,
                // Edge 2 at t=0
                edge1_vertex0_start, edge1_vertex1_start,
                // Edge 1 at t=1
                edge0_vertex0_end, edge0_vertex1_end,
                // Edge 2 at t=1
                edge1_vertex0_end, edge1_vertex1_end, min_distance);
        case CCDMethod::OURS:
        {
            throw "Trying to use OUR method, but it should not be used by this function";
        //     double toi;
            
        //     return intervalccd::edgeEdgeCCD_double(
        //  // Edge 1 at t=0
        //         edge0_vertex0_start, edge0_vertex1_start,
        //         // Edge 2 at t=0
        //         edge1_vertex0_start, edge1_vertex1_start,
        //         // Edge 1 at t=1
        //         edge0_vertex0_end, edge0_vertex1_end,
        //         // Edge 2 at t=1
        //         edge1_vertex0_end, edge1_vertex1_end,err,min_distance,toi,tolerance);
        }
        

        default:
            throw "Invalid Minimum Separation CCDMethod";
        }
    } catch (const char* err) {
        // Conservative answer upon failure.
        std::cerr << "Edge-edge CCD failed because \"" << err << "\" for "
                  << method_names[method] << std::endl;
        return true;
    } catch (...) {
        // Conservative answer upon failure.
        std::cerr << "Edge-edge CCD failed for unknown reason when using "
                  << method_names[method] << std::endl;
        return true;
    }
}
bool vertexFaceInterval(
    const Eigen::Vector3d& vertex_start,
    const Eigen::Vector3d& face_vertex0_start,
    const Eigen::Vector3d& face_vertex1_start,
    const Eigen::Vector3d& face_vertex2_start,
    const Eigen::Vector3d& vertex_end,
    const Eigen::Vector3d& face_vertex0_end,
    const Eigen::Vector3d& face_vertex1_end,
    const Eigen::Vector3d& face_vertex2_end){
        double toi;
        return intervalccd::vertexFaceCCD(
            // Point at t=0
                vertex_start,
                // Triangle at t = 0
                face_vertex0_start, face_vertex1_start, face_vertex2_start,
                // Point at t=1
                vertex_end,
                // Triangle at t = 1
                face_vertex0_end, face_vertex1_end, face_vertex2_end,
                toi
        );
    }
bool edgeEdgeInterval(
    const Eigen::Vector3d& edge0_vertex0_start,
    const Eigen::Vector3d& edge0_vertex1_start,
    const Eigen::Vector3d& edge1_vertex0_start,
    const Eigen::Vector3d& edge1_vertex1_start,
    const Eigen::Vector3d& edge0_vertex0_end,
    const Eigen::Vector3d& edge0_vertex1_end,
    const Eigen::Vector3d& edge1_vertex0_end,
    const Eigen::Vector3d& edge1_vertex1_end)
{
    double toi;// time of intersection
    return intervalccd::edgeEdgeCCD(
        edge0_vertex0_start,
        edge0_vertex1_start,
        edge1_vertex0_start,
        edge1_vertex1_start,
        edge0_vertex0_end,
        edge0_vertex1_end,
        edge1_vertex0_end,
        edge1_vertex1_end,toi);
    
}

} // namespace ccd
