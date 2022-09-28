#pragma once
#include "IPass.hpp"


namespace eureka::graphics
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

