#pragma once
#include <GPOPublicTypes.hpp>

#include <chrono>

// standard / 3rdParty types
namespace nlohmann 
{
    template <>
    struct adl_serializer<std::filesystem::path>
    {
        static void to_json(json& j, const std::filesystem::path& v)
        {
            j = v.string();
        }
    
        static void from_json(const json& j, std::filesystem::path& v)
        {
            v = j.get<std::string>();
        }
    };


    template <typename T, int m, int n>
    struct adl_serializer<Eigen::Matrix<T, m, n>>
    {
        static void to_json(json& j, const Eigen::Matrix<T, m, n>& mat)
        {
            j = json::array({});
            if constexpr (m == 1)
            {
                for (int k = 0; k < n; k++) j.push_back(mat(0, k));
            }
            else
            {
                for (int l = 0; l < m; l++)
                {
                    json jj = json::array();
                    for (int k = 0; k < n; k++) jj.push_back(mat(l, k));
                    j.push_back(jj);
                }
            }
        }

        static void from_json(const json& j, Eigen::Matrix<T, m, n>& mat)
        {
            int k = 0;
            for (auto it = j.begin(); it != j.end(); ++it, ++k)
            {
                if constexpr (m == 1)
                {
                    mat(0, k) = it->get<T>();
                }
                else
                {
                    int l = 0;
                    for (auto it2 = it->begin(); it2 != it->end(); ++it2, ++l)
                    {
                        mat(k, l) = it2->get<T>();
                    }
                }
            }
        }
    };

}

namespace nlohmann
{
    template <>
    struct adl_serializer<rgo::SE3Pose>
    {
        static void to_json(json& j, const rgo::SE3Pose& v)
        {
            j = json::array({});

            json j_t;
            json j_r;
            adl_serializer<Eigen::Vector3d>::to_json(j_t, v.translation());
            adl_serializer<Eigen::Matrix3d>::to_json(j_r, v.rotation());
            j.push_back(j_t);
            j.push_back(j_r);
        }

        static void from_json(const json& j, rgo::SE3Pose& v)
        {
            Eigen::Vector3d t;
            Eigen::Matrix3d r;
            adl_serializer<Eigen::Vector3d>::from_json(j.at(0), v.translation());
            adl_serializer<Eigen::Matrix3d>::from_json(j.at(1), v.rotation());
            v = rgo::SE3Pose(t, r);

        }
    };

    //
//template <typename T>
//struct adl_serializer<cv::Point_<T>> 
//{
//    static void to_json(json& j, const cv::Point_<T>& pt) 
//    {
//        j = json::array({pt.x, pt.y});
//    }
//
//    static void from_json(const json& j, cv::Point_<T>& pt) 
//    {
//        pt = cv::Point_<T>(j.at(0).get<T>(), (j.at(1).get<T>()));
//    }
//};
//
//template <typename T>
//struct adl_serializer<cv::Size_<T>> 
//{
//    static void to_json(json& j, const cv::Size_<T>& sz) 
//    {
//        j = json::array({sz.width, sz.height});
//    }
//
//    static void from_json(const json& j, cv::Size_<T>& sz) 
//    {
//        sz = cv::Size_<T>(j.at(0).get<T>(), j.at(1).get<T>());
//    }
//};
//
//template <typename T>
//struct adl_serializer<cv::Rect_<T>> 
//{
//    static void to_json(json& j, const cv::Rect_<T>& rect) 
//    {
//        j = json::array({rect.x, rect.y, rect.width, rect.height});
//    }
//
//    static void from_json(const json& j, cv::Rect_<T>& rect) 
//    {
//        rect = cv::Rect_<T>(
//            j.at(0).get<T>(), 
//            j.at(1).get<T>(), 
//            j.at(2).get<T>(), 
//            j.at(3).get<T>());
//    }
//};

    //template <>
    //struct adl_serializer<glm::vec3>
    //{
    //    static void to_json(json& j, const glm::vec3& v)
    //    {
    //        j = json::array({ v.x, v.y, v.z });
    //    }
	//
    //    static void from_json(const json& j, glm::vec3& v)
    //    {
    //        v.x = j.at(0).get<float>();
    //        v.y = j.at(1).get<float>();
    //        v.z = j.at(2).get<float>();
    //    }
    //};

    //template <>
    //struct adl_serializer<std::chrono::system_clock::time_point>
    //{
    //    static void to_json(json& j, const std::chrono::system_clock::time_point& t)
    //    {
    //        j = t.time_since_epoch().count() / 1'000;
    //    }
	//
    //    static void from_json(const json& j, std::chrono::system_clock::time_point& t)
    //    {
    //        t = std::chrono::time_point<std::chrono::system_clock>::min() + std::chrono::microseconds(j.get<int>());
    //    }
    //};


}