#define OTHER_LIBS interrogatedb \
                   dtoolutil:c dtoolbase:c dtool:m prc

#define USE_PACKAGES eigen

#begin lib_target
  #define TARGET linmath
  #define LOCAL_LIBS \
    express pandabase

  #define BUILDING_DLL BUILDING_PANDA_LINMATH

  #define SOURCES \
     aa_luse.h \
     compose_matrix.h compose_matrix_src.I  \
     compose_matrix_src.h config_linmath.h  \
     configVariableColor.h configVariableColor.I \
     coordinateSystem.h dbl2fltnames.h dblnames.h \
     deg_2_rad.h deg_2_rad.I \
     flt2dblnames.h fltnames.h \
     intnames.h \
     lcast_to.h lcast_to_src.h lcast_to_src.I  \
     lmatrix.h \
     lmat_ops.h lmat_ops_src.h lmat_ops_src.I \
     lmatrix3_src.h lmatrix3_src.I \
     lmatrix4_src.h lmatrix4_src.I \
     lorientation.h lorientation_src.h lorientation_src.I \
     lpoint2.h lpoint2_src.I lpoint2_src.h \
     lpoint3.h lpoint3_src.I lpoint3_src.h \
     lpoint4.h lpoint4_src.I lpoint4_src.h \
     lquaternion.h lquaternion_src.I \
     lquaternion_src.h lrotation.h lrotation_src.I \
     lrotation_src.h \
     lsimpleMatrix.h lsimpleMatrix.I \
     luse.I luse.N luse.h \
     lvec2_ops.h lvec2_ops_src.I lvec2_ops_src.h lvec3_ops.h \
     lvec3_ops_src.I lvec3_ops_src.h lvec4_ops.h lvec4_ops_src.I \
     lvec4_ops_src.h \
     lvecBase2.h lvecBase2_src.I lvecBase2_src.h \
     lvecBase3.h lvecBase3_src.I lvecBase3_src.h \
     lvecBase4.h lvecBase4_src.I lvecBase4_src.h \
     lvector2.h lvector2_src.I lvector2_src.h \
     lvector3.h lvector3_src.I lvector3_src.h \
     lvector4.h lvector4_src.I lvector4_src.h \
     mathNumbers.h mathNumbers.I

  #define COMPOSITE_SOURCES \
     compose_matrix.cxx config_linmath.cxx configVariableColor.cxx \
     coordinateSystem.cxx lmatrix.cxx \
     lorientation.cxx lpoint2.cxx  \
     lpoint3.cxx lpoint4.cxx lquaternion.cxx lrotation.cxx  \
     luse.cxx lvecBase2.cxx lvecBase3.cxx lvecBase4.cxx  \
     lvector2.cxx lvector3.cxx lvector4.cxx mathNumbers.cxx

  #define INSTALL_HEADERS \
     aa_luse.h \
     compose_matrix.h compose_matrix_src.I  \
     compose_matrix_src.h config_linmath.h  \
     configVariableColor.h configVariableColor.I \
     coordinateSystem.h dbl2fltnames.h dblnames.h \
     deg_2_rad.h deg_2_rad.I \
     flt2dblnames.h fltnames.h \
     intnames.h \
     lcast_to.h lcast_to_src.h lcast_to_src.I  \
     lmatrix.h \
     lmat_ops.h lmat_ops_src.h lmat_ops_src.I \
     lmatrix3_src.h lmatrix3_src.I \
     lmatrix4_src.h lmatrix4_src.I \
     lorientation.h lorientation_src.h lorientation_src.I \
     lpoint2.h lpoint2_src.I lpoint2_src.h \
     lpoint3.h lpoint3_src.I lpoint3_src.h \
     lpoint4.h lpoint4_src.I lpoint4_src.h \
     lquaternion.h lquaternion_src.I \
     lquaternion_src.h lrotation.h lrotation_src.I \
     lrotation_src.h \
     lsimpleMatrix.h lsimpleMatrix.I \
     luse.I luse.h \
     lvec2_ops.h lvec2_ops_src.I lvec2_ops_src.h lvec3_ops.h \
     lvec3_ops_src.I lvec3_ops_src.h lvec4_ops.h lvec4_ops_src.I \
     lvec4_ops_src.h \
     lvecBase2.h lvecBase2_src.I lvecBase2_src.h \
     lvecBase3.h lvecBase3_src.I lvecBase3_src.h \
     lvecBase4.h lvecBase4_src.I lvecBase4_src.h \
     lvector2.h lvector2_src.I lvector2_src.h \
     lvector3.h lvector3_src.I lvector3_src.h \
     lvector4.h lvector4_src.I lvector4_src.h \
     mathNumbers.h mathNumbers.I

  #define IGATESCAN all
  #define IGATEEXT \
    lmatrix_ext.h \
    lvecBase2_ext.h lvecBase3_ext.h lvecBase4_ext.h \
    lpoint2_ext.h lpoint3_ext.h lpoint4_ext.h \
    lvector2_ext.h lvector3_ext.h lvector4_ext.h

#end lib_target

#begin test_bin_target
  #define TARGET test_math
  #define LOCAL_LIBS \
    linmath

  #define SOURCES \
    test_math.cxx

#end test_bin_target
