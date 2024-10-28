file(GLOB GLSL_SOURCE_FILES "*.glsl")

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${CMAKE_BINARY_DIR}/shaders/spirv/${FILE_NAME}.spv")
  
  if (EMSCRIPTEN)
    set(GLSL_OUTPUT_DIR "${CMAKE_BINARY_DIR}/assets/shaders/glsl_100")
    set(GLSL_OUTPUT "${GLSL_OUTPUT_DIR}/${FILE_NAME}")
    set(GLSL_VERSION 300 --es)
  else()
    set(GLSL_OUTPUT_DIR "${CMAKE_BINARY_DIR}/assets/shaders/glsl_330")
    set(GLSL_OUTPUT "${GLSL_OUTPUT_DIR}/${FILE_NAME}")
    set(GLSL_VERSION 330)
  endif()

  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders/spirv")
  file(MAKE_DIRECTORY ${GLSL_OUTPUT_DIR})

  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND glslangValidator -S frag -G ${GLSL} -o ${SPIRV}
    MAIN_DEPENDENCY ${GLSL})

  add_custom_command(
    OUTPUT ${GLSL_OUTPUT}
    COMMAND spirv-cross --glsl-force-flattened-io-blocks --glsl-emit-ubo-as-plain-uniforms ${SPIRV} --output ${GLSL_OUTPUT} --version ${GLSL_VERSION} --no-420pack-extension
    MAIN_DEPENDENCY ${SPIRV})

  add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/assets/shaders/${FILE_NAME}"
    COMMAND ${CMAKE_COMMAND} -E copy ${GLSL_OUTPUT} "${CMAKE_SOURCE_DIR}/assets/shaders/${FILE_NAME}"
    MAIN_DEPENDENCY ${GLSL_OUTPUT})
    
  list(APPEND SHADER_FILES "${CMAKE_BINARY_DIR}/assets/shaders/${FILE_NAME}")
endforeach()

add_custom_target(
  ShaderConversion ALL
  DEPENDS ${SHADER_FILES})
