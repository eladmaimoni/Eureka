#pragma once


namespace eureka
{

    inline constexpr vk::Format DEFAULT_DEPTH_BUFFER_FORMAT = vk::Format::eD24UnormS8Uint;

    using CopySubmitExecutor = std::shared_ptr<concurrencpp::manual_executor>;
}