bin_PROGRAMS = zunda zunda-train zunda-conv zunda-fsem-train
zunda_SOURCES = main.cpp \
								modality.hpp \
								modality.cpp \
								sentence.hpp \
								sentence.cpp \
								feature.cpp \
								util.hpp \
								util.cpp \
								eval.hpp \
								cdbmap.hpp \
								funcsem.hpp \
								funcsem.cpp \
								../liblinear-1.8/linear.h \
								../cdbpp-1.1/include/cdbpp.h
zunda_LDADD = -L../tinyxml2 -ltinyxml2 -L../liblinear-1.8 -llinear -L../liblinear-1.8/blas -lblas @AM_LDFLAGS@ @BOOST_LIBS@

zunda_train_SOURCES = modality-learn.cpp \
											modality.hpp \
											modality.cpp \
											sentence.hpp \
											sentence.cpp \
											feature.cpp \
											util.hpp \
											util.cpp \
											eval.hpp \
											cdbmap.hpp \
											funcsem.hpp \
											funcsem.cpp \
											../liblinear-1.8/linear.h \
											../cdbpp-1.1/include/cdbpp.h
zunda_train_LDADD = -L../tinyxml2 -ltinyxml2 -L../liblinear-1.8 -llinear -L../liblinear-1.8/blas -lblas @AM_LDFLAGS@ @BOOST_LIBS@

zunda_conv_SOURCES = xml2cab.cpp \
										 modality.hpp \
										 modality.cpp \
										 sentence.hpp \
										 sentence.cpp \
										 feature.cpp \
										 util.hpp \
										 util.cpp \
										 cdbmap.hpp \
										 funcsem.hpp \
										 funcsem.cpp \
										 ../liblinear-1.8/linear.h \
										 ../cdbpp-1.1/include/cdbpp.h
zunda_conv_LDADD = -L../tinyxml2 -ltinyxml2 -L../liblinear-1.8 -llinear -L../liblinear-1.8/blas -lblas @AM_LDFLAGS@ @BOOST_LIBS@

zunda_fsem_train_SOURCES = funcsem-train.cpp \
													 funcsem.hpp \
													 funcsem.cpp \
													 sentence.hpp \
													 sentence.cpp \
													 util.hpp \
													 util.cpp
zunda_fsem_train_LDADD = @AM_LDFLAGS@ @BOOST_LIBS@

