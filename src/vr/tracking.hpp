#pragma once
#include "interaction/observation.hpp"
#include <openxr/openxr.h>
#include <memory>

namespace fantasynt::vr {
class Tracking {
public:
    Tracking(XrInstance instance,XrSystemId system,XrSession session,XrSpace local,bool hands_enabled,bool time_enabled);
    ~Tracking();
    interaction::ObservationFrame sample(XrTime time,std::uint64_t epoch,XrSessionState state,const char* status);
    nlohmann::json capabilities() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
