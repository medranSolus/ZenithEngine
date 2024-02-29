include_guard(DIRECTORY)

############# CS PERMUTATIONS ##############

# CACAO
add_shader_permutation("CACAOApplyCS" "ZE_CACAO_APPLY_METHOD=FFX_CACAO_NonSmartApply:NS|ZE_CACAO_APPLY_METHOD=FFX_CACAO_NonSmartHalfApply:NSF")
add_shader_permutation("CACAOApplyCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("CACAOBlurCS"
"ZE_CACAO_BLUR_METHOD=FFX_CACAO_EdgeSensitiveBlur2:2\
|ZE_CACAO_BLUR_METHOD=FFX_CACAO_EdgeSensitiveBlur3:3\
|ZE_CACAO_BLUR_METHOD=FFX_CACAO_EdgeSensitiveBlur4:4\
|ZE_CACAO_BLUR_METHOD=FFX_CACAO_EdgeSensitiveBlur5:5\
|ZE_CACAO_BLUR_METHOD=FFX_CACAO_EdgeSensitiveBlur6:6\
|ZE_CACAO_BLUR_METHOD=FFX_CACAO_EdgeSensitiveBlur7:7\
|ZE_CACAO_BLUR_METHOD=FFX_CACAO_EdgeSensitiveBlur8:8")
add_shader_permutation("CACAOBlurCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("CACAOBlurCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("CACAOGenerateCS"
"ZE_CACAO_GENERATE_METHOD=FFX_CACAO_GenerateQ1:Q1\
|ZE_CACAO_GENERATE_METHOD=FFX_CACAO_GenerateQ2,_ZE_CACAO_NORMAL_DIMMENSIONS:Q2\
|ZE_CACAO_GENERATE_METHOD=FFX_CACAO_GenerateQ3,_ZE_CACAO_NORMAL_DIMMENSIONS,_ZE_CACAO_ADDITIONAL_INPUT:Q3\
|ZE_CACAO_GENERATE_METHOD=FFX_CACAO_GenerateQ3Base,_ZE_CACAO_NORMAL_DIMMENSIONS:Q3B")
add_shader_permutation("CACAOGenerateCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("CACAOImportanceMapGenerateCS" "_ZE_PREFER_WAVE64:W")
add_shader_permutation("CACAOImportanceMapProcessACS" "_ZE_PREFER_WAVE64:W")
add_shader_permutation("CACAOImportanceMapProcessBCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("CACAOPrepareDepthCS" "_ZE_CACAO_PREPARE_DEPTH_HALF:F")
add_shader_permutation("CACAOPrepareDepthCS" "_ZE_CACAO_PREPARE_DOWNSAMPLED:D")
add_shader_permutation("CACAOPrepareDepthCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("CACAOPrepareDepthMipsCS" "_ZE_CACAO_PREPARE_DOWNSAMPLED:D")
add_shader_permutation("CACAOPrepareDepthMipsCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("CACAOPrepareNormalsCS" "_ZE_CACAO_PREPARE_NORMALS_INPUT:I")
add_shader_permutation("CACAOPrepareNormalsCS" "_ZE_CACAO_PREPARE_DOWNSAMPLED:D")
add_shader_permutation("CACAOPrepareNormalsCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("CACAOUpscaleCS" "FFX_CACAO_OPTION_APPLY_SMART=1:S")
add_shader_permutation("CACAOUpscaleCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("CACAOUpscaleCS" "_ZE_PREFER_WAVE64:W")

# Denoiser
add_shader_permutation("DenoiserReflectionsPrefilterCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("DenoiserReflectionsPrefilterCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("DenoiserReflectionsReprojectCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("DenoiserReflectionsReprojectCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("DenoiserReflectionsResolveCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("DenoiserReflectionsResolveCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("DenoiserShadowsClassificationCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("DenoiserShadowsClassificationCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("DenoiserShadowsFilterCS" "ZE_DENOISER_SHADOWS_FILTER_METHOD=DenoiserShadowsFilterPass1:1|ZE_DENOISER_SHADOWS_FILTER_METHOD=DenoiserShadowsFilterPass2:2")
add_shader_permutation("DenoiserShadowsFilterCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("DenoiserShadowsFilterCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("DenoiserShadowsPrepareMaskCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("DenoiserShadowsPrepareMaskCS" "_ZE_PREFER_WAVE64:W")

# FSR1
add_shader_permutation("FSR1EasuCS" "FFX_FSR1_OPTION_APPLY_RCAS=1:S")
add_shader_permutation("FSR1EasuCS" "FFX_FSR1_OPTION_SRGB_CONVERSIONS=1:R")
add_shader_permutation("FSR1EasuCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("FSR1EasuCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("FSR1RCasCS" "FFX_FSR1_OPTION_RCAS_PASSTHROUGH_ALPHA=1:A")
add_shader_permutation("FSR1RCasCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("FSR1RCasCS" "_ZE_PREFER_WAVE64:W")

# FSR2
add_shader_permutation("FSR2DepthClipCS" "FFX_FSR2_OPTION_INVERTED_DEPTH=1:I")
add_shader_permutation("FSR2DepthClipCS" "FFX_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS=1:R")
add_shader_permutation("FSR2DepthClipCS" "FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS=1:J")
add_shader_permutation("FSR2DepthClipCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("FSR2DepthClipCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("FSR2AccumulateCS" "FFX_FSR2_OPTION_APPLY_SHARPENING=1:S")
add_shader_permutation("FSR2AccumulateCS" "FFX_FSR2_OPTION_REPROJECT_USE_LANCZOS_TYPE=1:L")
add_shader_permutation("FSR2AccumulateCS" "FFX_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS=1:R")
add_shader_permutation("FSR2AccumulateCS" "FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS=1:J")
add_shader_permutation("FSR2AccumulateCS" "FFX_FSR2_OPTION_HDR_COLOR_INPUT=1:D")
add_shader_permutation("FSR2AccumulateCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("FSR2AccumulateCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("FSR2GenerateReactiveCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("FSR2GenerateReactiveCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("FSR2GenerateTCRCS" "FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS=1:J")
add_shader_permutation("FSR2GenerateTCRCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("FSR2GenerateTCRCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("FSR2LockCS" "FFX_FSR2_OPTION_INVERTED_DEPTH=1:I")
add_shader_permutation("FSR2LockCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("FSR2LockCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("FSR2LuminancePyramidCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("FSR2RCasCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("FSR2RCasCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("FSR2ReconstructPrevDepthCS" "FFX_FSR2_OPTION_INVERTED_DEPTH=1:I")
add_shader_permutation("FSR2ReconstructPrevDepthCS" "FFX_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS=1:R")
add_shader_permutation("FSR2ReconstructPrevDepthCS" "FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS=1:J")
add_shader_permutation("FSR2ReconstructPrevDepthCS" "FFX_FSR2_OPTION_HDR_COLOR_INPUT=1:D")
add_shader_permutation("FSR2ReconstructPrevDepthCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("FSR2ReconstructPrevDepthCS" "_ZE_PREFER_WAVE64:W")

# SSSR
add_shader_permutation("SSSRClassifyTilesCS" "_ZE_HALF_PRECISION:H")
add_shader_permutation("SSSRClassifyTilesCS" "_ZE_PREFER_WAVE64:W")

add_shader_permutation("SSSRDepthDownsampleCS" "_ZE_HALF_PRECISION:H")

add_shader_permutation("SSSRIntersectCS" "_ZE_PREFER_WAVE64:W")
add_shader_permutation("SSSRPrepareIndirectCS" "_ZE_PREFER_WAVE64:W")
add_shader_permutation("SSSRPrepareNoiseCS" "_ZE_PREFER_WAVE64:W")

# NIS
add_shader_permutation("NVImageScalingCS" "NIS_HDR_MODE=1:L|NIS_HDR_MODE=2:P")
add_shader_permutation("NVImageScalingCS" "NIS_USE_HALF_PRECISION=1:H|NIS_USE_HALF_PRECISION=1,NIS_BLOCK_HEIGHT=32,NIS_THREAD_GROUP_SIZE=128:NVH|NIS_THREAD_GROUP_SIZE=128:NV")

############# PS PERMUTATIONS ##############

add_shader_permutation("PbrPS" "_ZE_TRANSPARENT:T")
add_shader_permutation("PbrPS" "_ZE_USE_PARALLAX:P")
add_shader_permutation("PbrPS" "_ZE_OUTPUT_MOTION:M")
add_shader_permutation("PbrPS" "_ZE_OUTPUT_REACTIVE:R")

add_shader_permutation("ShadowPS" "_ZE_TRANSPARENT:T")
add_shader_permutation("ShadowPS" "_ZE_USE_PARALLAX:P")

add_shader_permutation("LightCombinePS" "_ZE_LIGHT_COMBINE_AO:A")

############# VS PERMUTATIONS ##############

add_shader_permutation("LambertVS" "_ZE_OUTPUT_MOTION:M")
