bin_PROGRAMS = zunda zunda-train
zunda_SOURCES = main.cpp \
								modality.hpp \
								modality.cpp \
								sentence.hpp \
								sentence.cpp \
								feature.cpp \
								util.hpp \
								eval.hpp
zunda_CXXFLAGS = @INCLUDES@ -DMODELDIR=\"${MODELDIR}\" -DDICDIR=\"${DICDIR}\"
zunda_LDADD = @LIBS@

zunda_train_SOURCES = modality-learn.cpp \
											modality.hpp \
											modality.cpp \
											sentence.hpp \
											sentence.cpp \
											feature.cpp \
											util.hpp \
											eval.hpp
zunda_train_CXXFLAGS=@INCLUDES@ -DMODELDIR=\"${MODELDIR}\" -DDICDIR=\"${DICDIR}\"
zunda_train_LDADD=@LIBS@

