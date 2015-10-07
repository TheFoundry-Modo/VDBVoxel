cmake_minimum_required(VERSION 3.0)

# Please overwrite include path variables to match your library paths.
# OpenVDB and VDBVoxel plugin should be compiled with the same versions of BOOST and TBB.

SET(BOOST_ROOT C:/dev/Temp/boost)
SET(MODO_SDK C:/dev/Temp/SDK)
SET(OPENVDB C:/dev/Temp/openvdb)
SET(TBB C:/dev/Temp/tbb44_20150728oss_win/tbb44_20150728oss)

INCLUDE_DIRECTORIES(
${BOOST_ROOT}
${MODO_SDK}/include
${OPENVDB}/..
${TBB}/include
)

ADD_DEFINITIONS(-DUNICODE -D_UNICODE)

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

ADD_LIBRARY (common 
${MODO_SDK}/common/clean.cpp
${MODO_SDK}/common/cwrap.cpp
${MODO_SDK}/common/lxu_attrdesc.cpp
${MODO_SDK}/common/lxu_attributes.cpp
${MODO_SDK}/common/lxu_command.cpp
${MODO_SDK}/common/lxu_deform.cpp
${MODO_SDK}/common/lxu_format.cpp
${MODO_SDK}/common/lxu_geometry.cpp
${MODO_SDK}/common/lxu_log.cpp
${MODO_SDK}/common/lxu_math.cpp
${MODO_SDK}/common/lxu_message.cpp
${MODO_SDK}/common/lxu_modifier.cpp
${MODO_SDK}/common/lxu_parser.cpp
${MODO_SDK}/common/lxu_prefvalue.cpp
${MODO_SDK}/common/lxu_queries.cpp
${MODO_SDK}/common/lxu_scene.cpp
${MODO_SDK}/common/lxu_select.cpp
${MODO_SDK}/common/lxu_simd.cpp
${MODO_SDK}/common/lxu_surface.cpp
${MODO_SDK}/common/lxu_tableau.cpp
${MODO_SDK}/common/lxu_vector.cpp
${MODO_SDK}/common/util.cpp)


ADD_LIBRARY (VDBVoxel SHARED
  ../../src/ClearCacheCommand.cpp
  ../../src/FileLoadCommand.cpp
  ../../src/OpenVDBCreation.cpp
  ../../src/OpenVDBGrid.cpp
  ../../src/OpenVDBItem.cpp
  ../../src/ParticleAttach.cpp
  ../../src/PopupCommand.cpp
  ../../src/VDBVoxelHash.cpp
  ../../src/VDBVoxelItem.cpp
  ../../src/VoxelItem.cpp
  ../../src/VoxelItemBin.cpp
  ../../src/VoxelItemCache.cpp
  ../../src/VoxelItemElement.cpp
  ../../src/VoxelItemInstance.cpp
  ../../src/VoxelItemPart.cpp
  ../../src/VoxelItemPref.cpp
  ../../src/VoxelItemSurface.cpp
)

SET_TARGET_PROPERTIES( VDBVoxel PROPERTIES COMPILE_DEFINITIONS "-D_USRDLL -DVDBVOXEL_EXPORTS")
SET_TARGET_PROPERTIES( VDBVoxel PROPERTIES SUFFIX ".lx")

IF(WIN32)
   SET_TARGET_PROPERTIES( VDBVoxel PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
   SET_TARGET_PROPERTIES( VDBVoxel PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
   SET_TARGET_PROPERTIES( VDBVoxel PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
   SET_TARGET_PROPERTIES( VDBVoxel PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
ENDIF(WIN32)

ADD_DEPENDENCIES ( VDBVoxel common )
