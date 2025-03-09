#include "response.hpp"

#include <userver/logging/log.hpp>

namespace osrm {
    namespace {
        int64_t GetIntFromJson(const userver::formats::json::Value& value) {
            if (!value.IsInt64() || value.IsNull() ) {
                return -1;
            }
            return value.As<int64_t>();
        }
    };
    // Response --------------------------------------------------------------------
    Response::Response(std::string&& body)
        : JsonResponse_(userver::formats::json::FromString(body))
    {
        Code_ = JsonResponse_["code"].As<std::string>();
        LOG_INFO() << "OSRM RESPONSE: " << body;
    }

    const std::string& Response::GetCode() const {
        return Code_;
    }
    // -----------------------------------------------------------------------------

    // ResponseRoute ---------------------------------------------------------------
    ResponseRoute::ResponseRoute(std::string&& body, const RequestRoute& request)
        : Response(std::move(body))
        , Request_(request)
    {
        Polyline_ = JsonResponse_["routes"][0]["geometry"].As<std::string>();
    }

    const std::string& ResponseRoute::GetPolyline() const {
        return Polyline_;
    }
    // -----------------------------------------------------------------------------

    // ResponseTable ---------------------------------------------------------------
    ResponseTable::ResponseTable(std::string&& body, const RequestTable& request)
        : Response(std::move(body))
        , Request_(request)
    {
        DurationTable_.reserve(Size_);
        DistanceTable_.reserve(Size_);
        for (uint64_t i = 0; i < Size_; i++) {
            DurationTable_[i].reserve(Size_);
            DistanceTable_[i].reserve(Size_);
            for (uint64_t j = 0; j < Size_; j++) {
                DurationTable_[i][j] = GetIntFromJson(JsonResponse_["durations"][i][j]);
                DistanceTable_[i][j] = GetIntFromJson(JsonResponse_["distances"][i][j]);
            }
        }
    }
    // -----------------------------------------------------------------------------
}
