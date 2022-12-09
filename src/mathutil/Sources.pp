#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#begin lib_target
  #define TARGET mathutil
  #define LOCAL_LIBS \
    linmath putil event express
  #define USE_PACKAGES fftw sleef
  #define UNIX_SYS_LIBS m

  #define BUILDING_DLL BUILDING_PANDA_MATHUTIL

  #define SOURCES  \
    boundingHexahedron.I boundingHexahedron.h boundingLine.I  \
    boundingLine.h \
    boundingBox.I boundingBox.h \
    boundingPlane.I boundingPlane.h \
    boundingSphere.I boundingSphere.h  \
    boundingVolume.I boundingVolume.h \
    colorRGBExp32.I colorRGBExp32.h \
    config_mathutil.h  \
    fftCompressor.h finiteBoundingVolume.h frustum.h  \
    frustum_src.I frustum_src.h geometricBoundingVolume.I \
    geometricBoundingVolume.h \
    interpolatedVariable.I interpolatedVariable.h \
    intersectionBoundingVolume.h intersectionBoundingVolume.I \
    lerpFunctions.h \
    linmath_events.h \
    look_at.h look_at_src.I \
    look_at_src.h \
    mathutil_avx_src.I mathutil_avx_src.h \
    mathutil_avx512_src.I mathutil_avx512_src.h \
    mathutil_misc.I mathutil_misc.T mathutil_misc.h \
    mathutil_simd.I mathutil_simd.h \
    mathutil_sse_src.I mathutil_sse_src.h \
    mersenne.h \
    omniBoundingVolume.I  \
    omniBoundingVolume.h \
    parabola.h parabola_src.I parabola_src.h \
    perlinNoise.h perlinNoise.I \
    perlinNoise2.h perlinNoise2.I \
    perlinNoise3.h perlinNoise3.I \
    plane.h plane_src.I plane_src.h \
    pta_LMatrix4.h pta_LMatrix3.h pta_LVecBase3.h \
    pta_LVecBase4.h pta_LVecBase2.h pta_LQuaternion.h \
    randomizer.h randomizer.I \
    rotate_to.h \
    stackedPerlinNoise2.h stackedPerlinNoise2.I \
    stackedPerlinNoise3.h stackedPerlinNoise3.I \
    triangulator.h triangulator.I \
    triangulator3.h triangulator3.I \
    triangulatorDelaunay.h triangulatorDelaunay.I \
    unionBoundingVolume.h unionBoundingVolume.I \
    winding.h winding.I

  #define COMPOSITE_SOURCES \
    boundingHexahedron.cxx boundingLine.cxx \
    boundingBox.cxx \
    boundingPlane.cxx \
    boundingSphere.cxx  \
    boundingVolume.cxx \
    colorRGBExp32.cxx \
    config_mathutil.cxx \
    fftCompressor.cxx  \
    finiteBoundingVolume.cxx geometricBoundingVolume.cxx  \
    interpolatedVariable.cxx \
    intersectionBoundingVolume.cxx \
    look_at.cxx \
    linmath_events.cxx \
    mathutil_misc.cxx \
    mathutil_simd.cxx \
    mersenne.cxx \
    omniBoundingVolume.cxx \
    parabola.cxx \
    perlinNoise.cxx \
    perlinNoise2.cxx \
    perlinNoise3.cxx \
    plane.cxx \
    pta_LMatrix4.cxx pta_LMatrix3.cxx pta_LVecBase3.cxx \
    pta_LVecBase4.cxx pta_LVecBase2.cxx pta_LQuaternion.cxx \
    randomizer.cxx \
    rotate_to.cxx \
    stackedPerlinNoise2.cxx \
    stackedPerlinNoise3.cxx \
    triangulator.cxx \
    triangulator3.cxx \
    triangulatorDelaunay.cxx \
    unionBoundingVolume.cxx \
    winding.cxx

  #define INSTALL_HEADERS \
    $[SOURCES]

  #define IGATESCAN all

  #define IGATEEXT \
    pta_LMatrix3_ext.h \
    pta_LMatrix4_ext.h \
    pta_LQuaternion_ext.h \
    pta_LVecBase2_ext.h \
    pta_LVecBase3_ext.h \
    pta_LVecBase4_ext.h

#end lib_target

#begin test_bin_target
  #define TARGET test_mathutil
  #define LOCAL_LIBS \
    mathutil pipeline

  #define SOURCES \
    test_mathutil.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_tri
  #define LOCAL_LIBS \
    mathutil pipeline

  #define SOURCES \
    test_tri.cxx

#end test_bin_target

#begin test_bin_target
  #define BUILD_TESTS 1
  #define TARGET test_simd
  #define LOCAL_LIBS mathutil
  #define SOURCES \
    test_simd.cxx

#end test_bin_target
