include ../../../mcl/common.mk
ifeq ($(UNAME_S),Darwin)
  JAVA_INC=-I/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers/
else
  JAVA_INC=-I/usr/lib/jvm/default-java/include
#JAVA_INC=-I/usr/lib/jvm/java-7-openjdk-amd64/include
  CFLAGS+=-z noexecstack
  LDFLAGS+=-lrt
endif
CFLAGS+=$(JAVA_INC) $(JAVA_INC)/linux -Wno-strict-aliasing
CFLAGS+=-std=c++03
SAMPLE_LIB=libsample.$(LIB_SUF)

PACKAGE_NAME=sample
PACKAGE_DIR=$(subst .,/,$(PACKAGE_NAME))

MCLJAVA_LIB=libmcljava.$(LIB_SUF)
JAVA_EXE=LD_LIBRARY_PATH=./ java -classpath ./

sample_wrap.cxx: sample.i
	$(MKDIR) $(PACKAGE_DIR)
	swig -java -package $(PACKAGE_NAME) -outdir $(PACKAGE_DIR) -c++ -Wall sample.i

$(SAMPLE_LIB): sample_wrap.cxx $(MCL_LIB)
	$(PRE)$(CXX) $< -o $@ $(CFLAGS) $(MCL_LIB) $(LDFLAGS) -shared

%.class: %.java
	javac $<

test: $(SAMPLE_LIB) SampleTest.class
	$(JAVA_EXE) SampleTest

clean:
	rm -rf *.class $(SAMPLE_LIB) *.class *_wrap.cxx sample

