#pragma once

namespace eureka
{

    struct ModelLoadingConfig
    {
       
    };
    struct LoadedModel
    {

    };

    LoadedModel LoadModel(const std::filesystem::path& path, const ModelLoadingConfig& config = ModelLoadingConfig{});
    
}