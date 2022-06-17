#pragma once
#include "IPass.hpp"
#include "Camera.hpp"


namespace eureka
{

    class SceneRenderer
    {
        std::shared_ptr<ITargetPass> _targetPass;
    public:
        SceneRenderer()
        {
            
        }
        void Prepare()
        {
            _targetPass->Prepare();
        }

        void RecordDraw(RecordParameters params)
        { 
            _targetPass->RecordDraw(params);
        }
    };


}

