#include "babylon.hpp"
#include <onnxruntime_cxx_api.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace VITS {
    Session::Session(const std::string& model_path) {
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "VITS");
        env.DisableTelemetryEvents();

        Ort::SessionOptions session_options;
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_DISABLE_ALL);
        session_options.DisableCpuMemArena();
        session_options.DisableMemPattern();
        session_options.DisableProfiling();

        session = new Ort::Session(env, model_path.c_str(), session_options);
    }

    Session::~Session() {
        delete session;
    }
}