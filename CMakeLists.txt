CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

SET(PROJECT_NAME "game")
PROJECT(${PROJECT_NAME} LANGUAGES CXX)

SET(CMAKE_C_STANDARD 20)
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_EXTENSIONS ON)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_VERBOSE_MAKEFILE ON)

IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

#SET(SANITIZERS -fsanitize=address,undefined,leak -fno-omit-frame-pointer -ftime-trace)
SET(SANITIZERS "")
#ADD_COMPILE_OPTIONS(-Wall ${SANITIZERS} -fno-plt -ftime-trace -ftime-report)
#ADD_LINK_OPTIONS(-Wl,--trace -Wl,--stats)

OPTION(USE_MOLD "Use mold linker" ON)
IF (USE_MOLD)
  EXECUTE_PROCESS(COMMAND mold --version OUTPUT_VARIABLE MOLD_VERSION)
  IF (MOLD_VERSION)
    MESSAGE(STATUS "Mold version is ${MOLD_VERSION}")
    ADD_LINK_OPTIONS(-fuse-ld=mold)
  ELSE()
    MESSAGE(ERROR "Mold not found")
  ENDIF()
ENDIF()

IF (EMSCRIPTEN)
  EXECUTE_PROCESS(
    COMMAND emcc --version
    OUTPUT_VARIABLE EMSCRIPTEN_VERSION_OUTPUT
  )

  STRING(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" EMSCRIPTEN_VERSION_MATCH ${EMSCRIPTEN_VERSION_OUTPUT})

  IF(EMSCRIPTEN_VERSION_MATCH)
    SET(EMSCRIPTEN_VERSION_MAJOR ${CMAKE_MATCH_1})
    SET(EMSCRIPTEN_VERSION_MINOR ${CMAKE_MATCH_2})
    SET(EMSCRIPTEN_VERSION_PATCH ${CMAKE_MATCH_3})
    MESSAGE(STATUS "Emscripten version is ${EMSCRIPTEN_VERSION_MAJOR}.${EMSCRIPTEN_VERSION_MINOR}.${EMSCRIPTEN_VERSION_PATCH}")

    IF(EMSCRIPTEN_VERSION_MAJOR GREATER_EQUAL 3 AND EMSCRIPTEN_VERSION_MINOR GREATER_EQUAL 1 AND EMSCRIPTEN_VERSION_PATCH GREATER_EQUAL 51)
      SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s GL_ENABLE_GET_PROC_ADDRESS ")
    ENDIF()
  ELSE()
    MESSAGE(WARNING "Unable to determine Emscripten version.")
  ENDIF()

  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 ")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASSERTIONS=0 ")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s WASM=1 ")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@assets")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ALLOW_MEMORY_GROWTH ")
  #SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s TOTAL_MEMORY=32MB ")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s EXPORTED_FUNCTIONS=\"['_main', '_malloc', '_free']\" ")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s MIN_WEBGL_VERSION=2 ")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASYNCIFY ")

  SET(CMAKE_EXECUTABLE_SUFFIX ".js")
  SET(PLATFORM "Web")

  CONFIGURE_FILE("src/main_page.html" "${CMAKE_BINARY_DIR}/src/index.html" COPYONLY)
ENDIF()

IF (APPLE)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} "-framework IOKit")
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} "-framework Cocoa")
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} "-framework OpenGL")
ENDIF()

ADD_SUBDIRECTORY(src)
