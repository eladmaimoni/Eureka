#pragma once
#include "Camera.hpp"


namespace eureka
{
    class IPass
    {
        virtual void Begin() = 0;
        virtual void Draw() = 0;
        virtual void End() = 0;
    };

    class IEffectPass
    {
        /*
        groups a set of objects to be rendered with the same shader / material, to the same target / view
        */
        //VkPipelineLayout layout;
        //std::array<VkDescriptorSetLayout, 4> setLayouts;
        //VkPipeline pipeline; // shared

        // 
        // bind descriptor sets that are shared by all objects
        // bind pipeline, optionally group objects by the same vertex buffer
        //
        virtual void Begin() = 0;
        // 
        // draw each object with the same effect
        // for (obj : object)
        //     Bind(object) // material descriptor sets, push constants, vertex data & offsets
        //     Draw(object)
        virtual void Draw() = 0;
        virtual void End() = 0;
    };

    struct ICameraPass  
    {
        /*
         groups the set of shader passes that uses the same characteristics
         - render target (render to target), or some deferred scheme
         - per view data (desc set 0)
         -
        */
        // set descriptor set 0 - per view data
        // 
        virtual void Begin() = 0;
        virtual void Draw() = 0;
        virtual void End() = 0;
        std::vector<IEffectPass> effectPasses;

    };

    struct ITargetPass // subpass or render pass
    {
        virtual void Begin() = 0;
        virtual void Draw() = 0;
        virtual void End() = 0;
        std::vector<ICameraPass> shaderPasses;
    };

    inline void RenderStuff()
    {


        /*


        for (targetPass : targetPasses) // each one is a vulkan subpass / render pass
        {
            targetPass.begin()
            for (cameraPass : targetPass.cameraPasses) // each one is a camera view on the same render target
            { // for instance ui pass is a different camera pass on the same target

                cameraPass.begin() // set descriptor set0, potentially others

                for (effectPass : cameraPass.effectPasses)
                {
                    effectPass.begin() // bind descriptor sets for material, bind vertex buffers if necessary

                    for (object : effectPass.objects)
                    {
                        effectPass.bind(object); // retrieve material data from object, set vertex buffer
                        effectPass.draw(object);
                    }

                    effectPass.end()
                }

                cameraPass.end()
            }
            targetPass.end()

        }
         // camera pass binding
        CameraPass.Begin() // bind set 0 per view

        for (subPasses : SubPass) // go over
        {
            for (pass : passGroup)
        }

        CameraPass.End()
        */
    }
}

