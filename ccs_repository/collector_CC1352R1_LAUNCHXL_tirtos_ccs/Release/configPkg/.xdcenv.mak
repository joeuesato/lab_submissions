#
_XDCBUILDCOUNT = 
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/source;C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/kernel/tirtos/packages
override XDCROOT = C:/ti/ccs1011/xdctools_3_61_02_27_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/source;C:/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54/kernel/tirtos/packages;C:/ti/ccs1011/xdctools_3_61_02_27_core/packages;..
HOSTOS = Windows
endif
