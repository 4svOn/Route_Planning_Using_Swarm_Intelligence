#include "response.hpp"

#include <userver/formats/json/serialize.hpp>

#include <userver/logging/log.hpp>

#include <sstream>

namespace osrm {
    namespace {
        int64_t GetIntFromJsonDouble(const userver::formats::json::Value& value) {
            if (value.IsInt64() && !value.IsNull()) {
                return value.As<int64_t>();
            }
            if (!value.IsDouble() || value.IsNull() ) {
                return -1;
            }
            return value.As<double>();
        }
    };

    // Response --------------------------------------------------------------------
    Response::Response(std::string&& body)
        : JsonResponse_(userver::formats::json::FromString(body))
    {
        Code_ = JsonResponse_["code"].As<std::string>();
        if (JsonResponse_.HasMember("message")) {
            Message_ = JsonResponse_["message"].As<std::string>();
        }
        LOG_INFO() << "OSRM RESPONSE: " << body;
    }

    std::string Response::GetLogString() const {
        std::stringstream ss;
        ss << "OSRM RESPONSE CODE: " << Code_ << "; MESSAGE: " << Message_;
        return ss.str();
    }

    bool Response::IsOk() const {
        return Code_ == "Ok";
    }
    // -----------------------------------------------------------------------------

    // ResponseRoute ---------------------------------------------------------------
    ResponseRoute::ResponseRoute(std::string&& body, const RequestRoute& request)
        : Response(std::move(body))
        , Request_(request)
    {
        if (!IsOk()) {
            LOG_ERROR() << GetLogString();
            return;
        }
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
        , Size_(request.GetSize())
        , DurationTable_(Size_, std::vector<int64_t>(Size_))
        , DistanceTable_(Size_, std::vector<int64_t>(Size_))
    {
        if (!IsOk()) {
            LOG_INFO() << GetLogString();
            return;
        }
        for (uint64_t i = 0; i < Size_; i++) {
            for (uint64_t j = 0; j < Size_; j++) {
                DurationTable_[i][j] = GetIntFromJsonDouble(JsonResponse_["durations"][i][j]);
                DistanceTable_[i][j] = GetIntFromJsonDouble(JsonResponse_["distances"][i][j]);
            }
        }
    }

    const RequestTable& ResponseTable::GetRequest() const {
        return Request_;
    }

    const uint64_t& ResponseTable::GetSize() const {
        return Size_;
    }

    const TDurationTable& ResponseTable::GetDurationTable() const {
        return DurationTable_;
    }

    const TDistanceTable& ResponseTable::GetDistanceTable() const {
        return DistanceTable_;
    }
    // -----------------------------------------------------------------------------
}
