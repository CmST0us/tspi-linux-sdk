#
# objects
#
ifneq ($(LOCAL_SRC_DIRS),)
    LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIRS) -name "*.c" -or -name "*.cpp" -or -name "*.cc")
endif
ifneq ($(LOCAL_SRC_DIRS_EXCLUDE),)
    LOCAL_SRC_FILES_EXCLUDE += $(shell find $(LOCAL_SRC_DIRS_EXCLUDE) -name "*.c" -or -name "*.cpp" -or -name "*.cc")
endif

LOCAL_SRC_FILES  := $(filter-out $(LOCAL_SRC_FILES_EXCLUDE), $(LOCAL_SRC_FILES))

OBJECTS = $(subst .c,.o,$(subst .cpp,.o,$(subst .cc,.o,$(LOCAL_SRC_FILES))))

#
# building targets
#
EXECUTABLE = $(LOCAL_MODULE)
SHARED_LIBRARY  = lib$(LOCAL_MODULE).so
STATIC_LIBRARY  = lib$(LOCAL_MODULE).a
PACKAGE  = $(shell basename .t/$(LOCAL_MODULE))-$(TARGET_PLATFORM)-$(shell uname -m)-$(VERSION)-$(TIMESTAMP).tar.gz

#
# explict rules
#
%.o : %.c
	$(CC) $(LOCAL_CFLAGS) $(CFLAGS) -c $< -o $@

%.o : %.cc
	$(CXX) $(LOCAL_CXXFLAGS) $(CXXFLAGS) -c $< -o $@

%.o : %.cpp
	$(CXX) $(LOCAL_CXXFLAGS) $(CXXFLAGS) -c $< -o $@


#
# goal: all
#
all: $(ALL)

$(EXECUTABLE) : $(OBJECTS)
	$(CXX) $^ $(LOCAL_LDFLAGS) $(LDFLAGS) -o $@

$(STATIC_LIBRARY) : $(OBJECTS)
	$(AR) crv $@ $^

$(SHARED_LIBRARY) : $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) $(LOCAL_LDFLAGS) -o $@
	$(STRIP) --strip-unneeded $@


#
# goal: clean
#
clean:
	rm -rf $(ALL) $(OBJECTS)


#
# goal: package
#
ifeq ($(findstring package,$(MAKECMDGOALS)),package)
    ifeq ($(VERSION),)
        $(error require argument 'VERSION' for 'package' goal)
    endif
    ifeq ($(PACKAGE_RESOURCES),)
    endif
endif

PACKAGE_TEMP_DIR = $(PACKAGE:.tar.gz=)
package: $(PACKAGE)
$(PACKAGE): $(ALL)
	@[ -e $(PACKAGE_TEMP_DIR) ] && echo "$(PACKAGE_TEMP_DIR) already exist, please delete it manually" && exit;\
	rm -rf $(PACKAGE_TEMP_DIR);\
	rm -rf $@;
	mkdir -p $(PACKAGE_TEMP_DIR);
	cp -rf -L $(ALL) $(LOCAL_PACKAGE_RESOURCES) $(PACKAGE_TEMP_DIR);
	tar --exclude .svn -h -czf $@ $(PACKAGE_TEMP_DIR);
	rm -rf $(PACKAGE_TEMP_DIR);
