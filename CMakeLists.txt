cmake_minimum_required(VERSION 3.13)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

# detect OS architecture
execute_process(
  COMMAND uname -s
  OUTPUT_VARIABLE UNAME_S
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

# Get Regenie version
file(STRINGS "VERSION" RG_VERSION)

project(regenie
  VERSION ${RG_VERSION}
  )

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Ensures -std=c++11

######################################
######## check input variables

# check for static compilation
if(BUILD_SHARED_LIBS)
  set(BUILD_STATIC OFF CACHE INTERNAL "Dynamic compilation")
  set(Boost_USE_STATIC_LIBS OFF)
else()
  set(BUILD_STATIC ON CACHE INTERNAL "Static compilation")
  set(Boost_USE_STATIC_LIBS ON)
  message( STATUS "Static compilation mode")
endif()

######################################
######## set flags and required libraries

set(BLA_STATIC               ${BUILD_STATIC})
set(Boost_USE_STATIC_LIBS    ${BUILD_STATIC})
set(Boost_USE_DEBUG_LIBS     OFF)
set(Boost_USE_MULTITHREADED  ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(MKLROOT "${CMAKE_PREFIX_PATH}")

# list each file specifically 
add_executable(regenie 
  ${CMAKE_SOURCE_DIR}/src/Data.cpp
  ${CMAKE_SOURCE_DIR}/src/Files.cpp
  ${CMAKE_SOURCE_DIR}/src/Geno.cpp
  ${CMAKE_SOURCE_DIR}/src/HLM.cpp
  ${CMAKE_SOURCE_DIR}/src/Interaction.cpp
  ${CMAKE_SOURCE_DIR}/src/Joint_Tests.cpp
  ${CMAKE_SOURCE_DIR}/src/Masks.cpp
  ${CMAKE_SOURCE_DIR}/src/NNLS.cpp
  ${CMAKE_SOURCE_DIR}/src/Pheno.cpp
  ${CMAKE_SOURCE_DIR}/src/Regenie.cpp
  ${CMAKE_SOURCE_DIR}/src/SKAT.cpp
  ${CMAKE_SOURCE_DIR}/src/Step1_Models.cpp
  ${CMAKE_SOURCE_DIR}/src/Step2_Models.cpp
  ${CMAKE_SOURCE_DIR}/src/MultiTrait_Tests.cpp
  ${CMAKE_SOURCE_DIR}/src/MCC.cpp
  ${CMAKE_SOURCE_DIR}/src/Ordinal.cpp
  ${CMAKE_SOURCE_DIR}/src/survival_data.cpp
  ${CMAKE_SOURCE_DIR}/src/cox_ridge.cpp
  ${CMAKE_SOURCE_DIR}/src/cox_score.cpp
  ${CMAKE_SOURCE_DIR}/src/cox_firth.cpp
  )
target_include_directories(regenie PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_PREFIX_PATH}/include)

set(CMAKE_CXX_FLAGS "-O3 -Wall -pedantic -ffast-math -Wno-unused-local-typedefs -Wno-deprecated-declarations -Wno-long-long -fPIC")

find_package(OpenMP REQUIRED)
target_link_libraries(regenie OpenMP::OpenMP_CXX)
if("${UNAME_S}" STREQUAL "Linux")
  if(${BUILD_STATIC})
    target_link_options(regenie BEFORE -static-libgcc -static-libstdc++)
  endif()
elseif("${UNAME_S}" STREQUAL "Darwin")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

set(EXTERN_LIBS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external_libs")
target_include_directories(regenie PUBLIC ${EXTERN_LIBS_PATH}/)

# for BGEN (choose static if present first)
find_library(ZSTD_LIBRARY zstd REQUIRED)
find_library(DB_LIBRARY 
  NAMES libdb.a db 
  REQUIRED
  )
find_library(SQLITE3_LIBRARY sqlite3 REQUIRED)
find_package(Boost 
  REQUIRED COMPONENTS system filesystem thread 
  OPTIONAL_COMPONENTS iostreams
  )
find_library(BGEN_LIBRARY 
  NAMES libbgen.a 
  REQUIRED
  )
target_link_libraries(regenie ${ZSTD_LIBRARY} ${BGEN_LIBRARY} ${DB_LIBRARY} ${SQLITE3_LIBRARY} ${Boost_LIBRARIES})
target_include_directories(regenie 
  PUBLIC
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/genfile>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/db>
  $<INSTALL_INTERFACE:${BOOST_INCLUDEDIR}>
  )
add_definitions(${Boost_DEFINITIONS})

# Boost IO
if(Boost_iostreams_FOUND) 
  add_definitions(-DHAS_BOOST_IOSTREAM)
  set(RG_VERSION "${RG_VERSION}.gz" CACHE INTERNAL "Update version")
  message( STATUS "Will compile with Boost Iostreams library")
endif()

# MVTNorm library
set(MVTN_PATH "${EXTERN_LIBS_PATH}/mvtnorm")
add_custom_target(
   libMvtnorm
   COMMAND make
   WORKING_DIRECTORY ${MVTN_PATH}
)
target_link_libraries(regenie ${MVTN_PATH}/libMvtnorm.a)
add_dependencies(regenie libMvtnorm)

# QF library
set(QF_PATH "${EXTERN_LIBS_PATH}/qf")
add_custom_target(
   libqf
   COMMAND make
   WORKING_DIRECTORY ${QF_PATH}
)
target_link_libraries(regenie ${QF_PATH}/qf.a)
add_dependencies(regenie libqf)

# Quadpack library
set(QUAD_PATH "${EXTERN_LIBS_PATH}/quadpack")
add_custom_target(
   libquad
   COMMAND make
   WORKING_DIRECTORY ${QUAD_PATH}
)
target_link_libraries(regenie ${QUAD_PATH}/libquad.a)
add_dependencies(regenie libquad)

# PGEN library
set(PGEN_PATH "${EXTERN_LIBS_PATH}/pgenlib")
add_custom_target(
   pgenlib
   COMMAND make
   WORKING_DIRECTORY ${PGEN_PATH}
)
target_link_libraries(regenie ${PGEN_PATH}/pgenlib.a)
target_include_directories(regenie PUBLIC ${PGEN_PATH} ${PGEN_PATH}/simde/ ${PGEN_PATH}/include/)
add_dependencies(regenie pgenlib)

# REMETA library
if(DEFINED HTSLIB_PATH)
  set(REMETA_PATH "${EXTERN_LIBS_PATH}/remeta")
  add_custom_target(
     remeta
     COMMAND make HTSLIB_PATH=${HTSLIB_PATH}
     WORKING_DIRECTORY ${REMETA_PATH}
  )
  target_link_libraries(regenie PUBLIC ${REMETA_PATH}/remeta.a)
  target_include_directories(regenie PUBLIC ${REMETA_PATH})
  add_dependencies(regenie remeta)

  add_definitions(-DWITH_HTSLIB)
  find_library(HTSLIB libhts.a HINTS ${HTSLIB_PATH})
  find_library(BZ2_LIB bz2 REQUIRED)
  find_library(LZMA_LIB lzma REQUIRED)
  find_library(CURL_LIB curl REQUIRED)
  find_library(CRYPTO_LIB crypto REQUIRED)
  target_link_libraries(regenie PUBLIC 
    ${HTSLIB}
    ${BZ2_LIB} ${LZMA_LIB} ${CURL_LIB} ${CRYPTO_LIB} 
  )
endif()

# Intel MKL
if(DEFINED MKLROOT) 
  add_definitions(-DWITH_MKL -DEIGEN_USE_BLAS -DEIGEN_USE_LAPACKE)
  target_include_directories(regenie PUBLIC ${MKLROOT}/include/)
  find_library(MKL_LP64_LIB mkl_intel_lp64 
    HINTS "${MKLROOT}/lib/intel64" 
    "${MKLROOT}/lib" 
    REQUIRED
    )
  if("${UNAME_S}" STREQUAL "Linux")
    find_library(MKL_THREAD_LIB mkl_gnu_thread 
      HINTS "${MKLROOT}/lib/intel64" 
      "${MKLROOT}/lib" 
      REQUIRED
      )
  elseif("${UNAME_S}" STREQUAL "Darwin")
    find_library(MKL_THREAD_LIB mkl_intel_thread 
      HINTS "${MKLROOT}/lib/intel64" 
      "${MKLROOT}/lib" 
      REQUIRED
      )
  endif()
  find_library(MKL_CORE_LIB mkl_core 
    HINTS "${MKLROOT}/lib/intel64" 
    "${MKLROOT}/lib" 
    REQUIRED
    )
  if("${UNAME_S}" STREQUAL "Darwin")
    target_link_libraries(regenie ${MKL_LP64_LIB} ${MKL_THREAD_LIB} ${MKL_CORE_LIB})
  elseif(${BUILD_STATIC})
    target_link_libraries(regenie "-Wl,--start-group" ${MKL_LP64_LIB} ${MKL_THREAD_LIB} ${MKL_CORE_LIB} "-Wl,--end-group" -lgomp)
  else() # use dynamic libs
    target_link_libraries(regenie "-Wl,--no-as-needed" ${MKL_LP64_LIB} ${MKL_THREAD_LIB} ${MKL_CORE_LIB} -lgomp)
  endif()
  message( STATUS "Will compile with Intel MKL library")
endif()

# cxxopts (header-only)
target_include_directories(regenie PUBLIC ${CMAKE_SOURCE_DIR}/external_libs/cxxopts/include/)

# LBFGS (header-only)
target_include_directories(regenie PUBLIC ${CMAKE_SOURCE_DIR}/external_libs/LBFGSpp/include/)

# Eigen (header-only)
target_include_directories(regenie PUBLIC ${CMAKE_SOURCE_DIR}/external_libs/eigen-3.4.0/)

# Other libraries
find_library(ZLIB_LIBRARY z REQUIRED)
find_library(M_LIB m REQUIRED) 
find_library(DL_LIB dl REQUIRED) 
if("${UNAME_S}" STREQUAL "Linux")
  set(GFORTRAN_LIBRARY "-lgfortran")
elseif("${UNAME_S}" STREQUAL "Darwin")
  find_library(GFORTRAN_LIBRARY gfortran REQUIRED)
endif()
target_link_libraries(regenie ${ZLIB_LIBRARY} ${M_LIB} ${DL_LIB} ${PTHREAD_LIB} ${GFORTRAN_LIBRARY})

add_definitions(-DVERSION_NUMBER="${RG_VERSION}")
install(TARGETS regenie RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
message( STATUS "REGENIE v" ${RG_VERSION})

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake DESTINATION share/${PROJECT_NAME})

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
include(CPack)
