#pragma once
#include "PipelineTypes.hpp"
#include <boost/hana/adapt_struct.hpp>


BOOST_HANA_ADAPT_STRUCT(eureka::ImGuiVertex, position, uv, color);
BOOST_HANA_ADAPT_STRUCT(eureka::PositionColorVertex, position, color);

namespace eureka
{


}

