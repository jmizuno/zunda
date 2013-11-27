bin_PROGRAMS = zunda zunda-train zunda-conv
zunda_SOURCES = main.cpp \
								modality.hpp \
								modality.cpp \
								sentence.hpp \
								sentence.cpp \
								feature.cpp \
								util.hpp \
								eval.hpp \
								cdbmap.hpp \
								../liblinear-1.8/linear.h \
								../cdbpp-1.1/include/cdbpp.h
zunda_CXXFLAGS = @INCLUDES@ -DMODELDIR_IPA=\"${MODELDIR_IPA}\" -DMODELDIR_JUMAN=\"${MODELDIR_JUMAN}\" -DDICDIR=\"${DICDIR}\"
zunda_LDADD = -L../tinyxml2 -ltinyxml2 -L../liblinear-1.8 -llinear -L../liblinear-1.8/blas -lblas @LIBS@

zunda_train_SOURCES = modality-learn.cpp \
											modality.hpp \
											modality.cpp \
											sentence.hpp \
											sentence.cpp \
											feature.cpp \
											util.hpp \
											eval.hpp \
											cdbmap.hpp \
											../liblinear-1.8/linear.h \
											../cdbpp-1.1/include/cdbpp.h
zunda_train_CXXFLAGS = @INCLUDES@ -DMODELDIR_IPA=\"${MODELDIR_IPA}\" -DMODELDIR_JUMAN=\"${MODELDIR_JUMAN}\" -DDICDIR=\"${DICDIR}\"
zunda_train_LDADD = -L../tinyxml2 -ltinyxml2 -L../liblinear-1.8 -llinear -L../liblinear-1.8/blas -lblas @LIBS@
zunda_conv_SOURCES = xml2cab.cpp \
										 modality.hpp \
										 modality.cpp \
										 sentence.hpp \
										 sentence.cpp \
										 feature.cpp \
										 util.hpp \
										 cdbmap.hpp \
										 ../liblinear-1.8/linear.h \
										 ../cdbpp-1.1/include/cdbpp.h
zunda_conv_LDADD = -L../tinyxml2 -ltinyxml2 -L../liblinear-1.8 -llinear -L../liblinear-1.8/blas -lblas @LIBS@

