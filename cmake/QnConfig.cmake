include(CMakeFindDependencyMacro)
find_dependency(ROOT COMPONENTS Core MathCore RIO Hist Tree Net TreePlayer)
include("${CMAKE_CURRENT_LIST_DIR}/QnTargets.cmake")