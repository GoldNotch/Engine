#include <Formatter.hpp>
#include <vulkan/vulkan.hpp>

#include "Builders.hpp"
#include "ShaderCompiler.hpp"

namespace
{

VkShaderStageFlagBits ShaderType2ShaderStageFlag(RHI::ShaderType type)
{
  constexpr static VkShaderStageFlagBits mapping[] = {VK_SHADER_STAGE_VERTEX_BIT,
                                                      VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                      VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                                      VK_SHADER_STAGE_GEOMETRY_BIT,
                                                      VK_SHADER_STAGE_FRAGMENT_BIT,
                                                      VK_SHADER_STAGE_COMPUTE_BIT};
  return mapping[static_cast<size_t>(type)];
}

} // namespace

namespace RHI::vulkan::details
{
vk::DescriptorSetLayout DescriptorSetLayoutBuilder::Make(const vk::Device & device) const
{
  // create descriptor set layout
  VkDescriptorSetLayoutCreateInfo dsetLayoutInfo{};
  dsetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  dsetLayoutInfo.bindingCount = static_cast<uint32_t>(m_descriptorsLayout.size());
  dsetLayoutInfo.pBindings = m_descriptorsLayout.data();

  VkDescriptorSetLayout descr_layout;
  if (auto res = vkCreateDescriptorSetLayout(device, &dsetLayoutInfo, nullptr, &descr_layout);
      res != VK_SUCCESS)
    throw std::runtime_error(Formatter() << "Failed to create descriptor set layout - " << res);
  return vk::DescriptorSetLayout(descr_layout);
}

void DescriptorSetLayoutBuilder::Reset()
{
  m_descriptorsLayout.clear();
}

} // namespace RHI::vulkan::details

namespace RHI::vulkan::details
{

vk::PipelineLayout PipelineLayoutBuilder::Make(const vk::Device & device) const
{
  // create pipeline layout
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_layouts.size()); // Optional
  pipelineLayoutInfo.pSetLayouts = m_layouts.data();                           // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0;                               // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr;                            // Optional

  VkPipelineLayout layout;
  if (auto res = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout);
      res != VK_SUCCESS)
    throw std::runtime_error(Formatter() << "Failed to create pipeline layout - " << res);
  return layout;
}

} // namespace RHI::vulkan::details


namespace RHI::vulkan::details
{
PipelineBuilder::PipelineBuilder()
{
  m_dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

  auto && color_blend_attachment = m_colorBlendAttachments.emplace_back();
  color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable = VK_FALSE;
  color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
  color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
  color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
  color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional
}


vk::Pipeline PipelineBuilder::Make(const vk::Device & device, const VkRenderPass & renderPass,
                                   uint32_t subpass_index, const VkPipelineLayout & layout)
{
  VkPipelineDynamicStateCreateInfo dynamicStatesInfo{};
  dynamicStatesInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStatesInfo.dynamicStateCount = static_cast<uint32_t>(m_dynamicStates.size());
  dynamicStatesInfo.pDynamicStates = m_dynamicStates.data();

  // vertex input format
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(m_bindings.size());
  vertexInputInfo.pVertexBindingDescriptions = m_bindings.data(); // Optional
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributes.size());
  vertexInputInfo.pVertexAttributeDescriptions = m_attributes.data(); // Optional

  // GL_TRIANGLES, GL_POINTS, etc
  VkPipelineInputAssemblyStateCreateInfo assemblyInfo{};
  assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  assemblyInfo.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportInfo{};
  viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportInfo.viewportCount = 1;
  viewportInfo.scissorCount = 1;

  VkPipelineDepthStencilStateCreateInfo m_depthStencilInfo{};


  // Culling, polygone mode, linewidth, culling
  VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
  rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizationInfo.depthClampEnable = VK_FALSE;
  rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterizationInfo.lineWidth = m_lineWidth;
  rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizationInfo.depthBiasEnable = VK_FALSE;
  rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
  rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
  rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional

  VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
  multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisamplingInfo.sampleShadingEnable = VK_FALSE;
  multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisamplingInfo.minSampleShading = 1.0f;          // Optional
  multisamplingInfo.pSampleMask = nullptr;            // Optional
  multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
  multisamplingInfo.alphaToOneEnable = VK_FALSE;      // Optional

  VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
  colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlendInfo.logicOpEnable = VK_FALSE;
  colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlendInfo.blendConstants[0] = 0.0f;   // Optional
  colorBlendInfo.blendConstants[1] = 0.0f;   // Optional
  colorBlendInfo.blendConstants[2] = 0.0f;   // Optional
  colorBlendInfo.blendConstants[3] = 0.0f;   // Optional
  colorBlendInfo.attachmentCount = static_cast<uint32_t>(m_colorBlendAttachments.size());
  colorBlendInfo.pAttachments = m_colorBlendAttachments.data();

  // Build shaders
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  std::vector<vk::ShaderModule> compiledShaders;
  for (auto && [type, path] : m_attachedShaders)
  {
    auto && module = BuildShaderModule(device, path);
    compiledShaders.push_back(module);
    VkPipelineShaderStageCreateInfo & info =
      shaderStages.emplace_back(VkPipelineShaderStageCreateInfo{});
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.stage = ::ShaderType2ShaderStageFlag(type);
    info.pName = "main";
    info.module = module;
  }

  // create pipeline
  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.layout = layout;
  pipeline_info.renderPass = renderPass;
  pipeline_info.subpass = subpass_index;
  //states
  pipeline_info.stageCount = static_cast<uint32_t>(shaderStages.size());
  pipeline_info.pStages = shaderStages.data();
  pipeline_info.pDynamicState = &dynamicStatesInfo;
  pipeline_info.pInputAssemblyState = &assemblyInfo;
  pipeline_info.pVertexInputState = &vertexInputInfo;
  pipeline_info.pViewportState = &viewportInfo;
  pipeline_info.pRasterizationState = &rasterizationInfo;
  pipeline_info.pMultisampleState = &multisamplingInfo;
  pipeline_info.pColorBlendState = &colorBlendInfo;
  pipeline_info.pDepthStencilState = nullptr;        // Optional
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipeline_info.basePipelineIndex = -1;              // Optional

  VkPipeline pipeline{};
  if (auto res =
        vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);
      res != VK_SUCCESS)
    throw std::runtime_error(Formatter() << "Failed to create graphics pipeline - " << res);

  // cleanup shader modules
  for (auto && shader : compiledShaders)
    vkDestroyShaderModule(device, shader, nullptr);

  // initialize object
  //for (auto && layout_info : m_descriptorsLayout)
  //  result->CreateUniformDescriptors(layout_info.descriptorType, layout_info.descriptorCount);
  //return result;
}

void PipelineBuilder::Reset()
{
  m_lineWidth = 1.0f;
  m_polygonMode = PolygonMode::Fill;
  m_cullingMode = CullingMode::None;
  m_frontFace = FrontFace::CCW;

  m_blendEnabled = false;
  m_blendColorOp = BlendOperation::Add;
  m_blendAlphaOp = BlendOperation::Add;
  m_blendSrcColorFactor = BlendFactor::One;
  m_blendDstColorFactor = BlendFactor::Zero;
  m_blendSrcAlphaFactor = BlendFactor::One;
  m_blendDstAlphaFactor = BlendFactor::Zero;
}

void PipelineBuilder::AttachShader(RHI::ShaderType type, const std::filesystem::path & path)
{
  m_attachedShaders.push_back({type, path});
}

} // namespace RHI::vulkan::details
