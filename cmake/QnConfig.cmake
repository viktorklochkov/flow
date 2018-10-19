include(CMakeFindDependencyMacro)
find_depedency(ROOT REQUIRED Core MathCore RIO Hist Tree Net TreePlayer)
include("${CMAKE_CURRENT_LIST_DIR}/QnTargets.cmake")