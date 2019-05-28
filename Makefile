ARCH_FLAGS_MK=

GENLINK_VARS = ARCH_FLAGS_MK CPPFLAGS_MK
ifneq ($(filter undefined,$(foreach VAR,$(GENLINK_VARS),$(origin $(VAR)))),)
$(info something is not defined)
endif

all:


BLA = a b c

$(foreach V,$(BLA),$(eval BLU:=$(V) $(BLU)))

$(info BLU=$(BLU))


BLE=a.a b.b c.c
XY=a c
$(info $(filter $(addprefix %,$(XY)),$(BLE)))

D=. tools
$(info $(realpath $D))

$(info TARGET=$(TARGET))

$(info noreal=$(realpath blablabla))
