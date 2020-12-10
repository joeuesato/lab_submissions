################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1011/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/bin/armcl" --cmd_file="C:/Users/uesatj1/Documents/GitHub/lab_submissions/ccs_repository/collector_CC1352R1_LAUNCHXL_tirtos_ccs/application/defines/collector.opts"  -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O4 --opt_for_speed=0 --include_path="C:/Users/uesatj1/Documents/GitHub/lab_submissions/ccs_repository/collector_CC1352R1_LAUNCHXL_tirtos_ccs" --include_path="C:/Users/uesatj1/Documents/GitHub/lab_submissions/ccs_repository/collector_CC1352R1_LAUNCHXL_tirtos_ccs/Release" --include_path="C:/Users/uesatj1/Documents/GitHub/lab_submissions/ccs_repository/collector_CC1352R1_LAUNCHXL_tirtos_ccs/application/collector" --include_path="C:/Users/uesatj1/Documents/GitHub/lab_submissions/ccs_repository/collector_CC1352R1_LAUNCHXL_tirtos_ccs/application" --include_path="C:/Users/uesatj1/Documents/GitHub/lab_submissions/ccs_repository/collector_CC1352R1_LAUNCHXL_tirtos_ccs/software_stack/ti15_4stack/osal" --include_path="/source/ti/ti154stack/apps" --include_path="/source/ti/ti154stack/common" --include_path="/source/ti/ti154stack/common/boards" --include_path="/source/ti/ti154stack/common/osal_port" --include_path="/source/ti/ti154stack/common/util" --include_path="/source/ti/ti154stack/common/inc" --include_path="/source/ti/ti154stack/common/stack/src" --include_path="/source/ti/ti154stack/common/stack/tirtos/inc" --include_path="/source/ti/ti154stack/common/heapmgr" --include_path="/source/ti/ti154stack/services/saddr" --include_path="/source/ti/ti154stack/services/sdata" --include_path="/source/ti/ti154stack/hal/crypto" --include_path="/source/ti/ti154stack/hal/platform" --include_path="/source/ti/ti154stack/hal/rf" --include_path="/source/ti/ti154stack/fh" --include_path="/source/ti/ti154stack/high_level" --include_path="/source/ti/ti154stack/inc" --include_path="/source/ti/ti154stack/rom" --include_path="/source/ti/ti154stack/inc/cc13xx" --include_path="/source/ti/ti154stack/low_level/cc13xx" --include_path="/source/ti/ti154stack/tracer" --include_path="/source/ti/ti154stack/apps/collector" --include_path="/source/ti/devices/cc13x2_cc26x2" --include_path="/source/ti/devices/cc13x2_cc26x2/inc" --include_path="/source/ti/devices/cc13x2_cc26x2/driverlib" --include_path="/source/ti/common/nv" --include_path="/source/ti/common/cui" --include_path="/source/ti/posix/ccs" --include_path="C:/ti/ccs1011/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/include" --define=TIMAC_ROM_IMAGE_BUILD --define=Board_EXCLUDE_NVS_EXTERNAL_FLASH --define=DeviceFamily_CC13X2 -g --c99 --plain_char=unsigned --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --embedded_constants=on --unaligned_access=on --enum_type=packed --wchar_t=16 --common=on --fp_reassoc=off --sat_reassoc=off --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" --include_path="C:/Users/uesatj1/Documents/GitHub/lab_submissions/ccs_repository/collector_CC1352R1_LAUNCHXL_tirtos_ccs/Release/syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1614026113:
	@$(MAKE) --no-print-directory -Onone -f subdir_rules.mk build-1614026113-inproc

build-1614026113-inproc: ../app.cfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: XDCtools'
	"C:/ti/ccs1011/xdctools_3_61_02_27_core/xs" --xdcpath= xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M4F -p ti.platforms.simplelink:CC1352R1F3 -r release -c "C:/ti/ccs1011/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS" --compileOptions "-mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O4 --opt_for_speed=0 --include_path=\"C:/Users/uesatj1/Documents/GitHub/lab_submissions/ccs_repository/collector_CC1352R1_LAUNCHXL_tirtos_ccs\" --include_path=\"C:/Users/uesatj1/Documents/GitHub/lab_submissions/ccs_repository/collector_CC1352R1_LAUNCHXL_tirtos_ccs/Release\" --include_path=\"C:/Users/uesatj1/Documents/GitHub/lab_submissions/ccs_repository/collector_CC1352R1_LAUNCHXL_tirtos_ccs/application/collector\" --include_path=\"C:/Users/uesatj1/Documents/GitHub/lab_submissions/ccs_repository/collector_CC1352R1_LAUNCHXL_tirtos_ccs/application\" --include_path=\"C:/Users/uesatj1/Documents/GitHub/lab_submissions/ccs_repository/collector_CC1352R1_LAUNCHXL_tirtos_ccs/software_stack/ti15_4stack/osal\" --include_path=\"/source/ti/ti154stack/apps\" --include_path=\"/source/ti/ti154stack/common\" --include_path=\"/source/ti/ti154stack/common/boards\" --include_path=\"/source/ti/ti154stack/common/osal_port\" --include_path=\"/source/ti/ti154stack/common/util\" --include_path=\"/source/ti/ti154stack/common/inc\" --include_path=\"/source/ti/ti154stack/common/stack/src\" --include_path=\"/source/ti/ti154stack/common/stack/tirtos/inc\" --include_path=\"/source/ti/ti154stack/common/heapmgr\" --include_path=\"/source/ti/ti154stack/services/saddr\" --include_path=\"/source/ti/ti154stack/services/sdata\" --include_path=\"/source/ti/ti154stack/hal/crypto\" --include_path=\"/source/ti/ti154stack/hal/platform\" --include_path=\"/source/ti/ti154stack/hal/rf\" --include_path=\"/source/ti/ti154stack/fh\" --include_path=\"/source/ti/ti154stack/high_level\" --include_path=\"/source/ti/ti154stack/inc\" --include_path=\"/source/ti/ti154stack/rom\" --include_path=\"/source/ti/ti154stack/inc/cc13xx\" --include_path=\"/source/ti/ti154stack/low_level/cc13xx\" --include_path=\"/source/ti/ti154stack/tracer\" --include_path=\"/source/ti/ti154stack/apps/collector\" --include_path=\"/source/ti/devices/cc13x2_cc26x2\" --include_path=\"/source/ti/devices/cc13x2_cc26x2/inc\" --include_path=\"/source/ti/devices/cc13x2_cc26x2/driverlib\" --include_path=\"/source/ti/common/nv\" --include_path=\"/source/ti/common/cui\" --include_path=\"/source/ti/posix/ccs\" --include_path=\"C:/ti/ccs1011/ccs/tools/compiler/ti-cgt-arm_20.2.1.LTS/include\" --define=TIMAC_ROM_IMAGE_BUILD --define=Board_EXCLUDE_NVS_EXTERNAL_FLASH --define=DeviceFamily_CC13X2 -g --c99 --plain_char=unsigned --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --embedded_constants=on --unaligned_access=on --enum_type=packed --wchar_t=16 --common=on --fp_reassoc=off --sat_reassoc=off  " "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

configPkg/linker.cmd: build-1614026113 ../app.cfg
configPkg/compiler.opt: build-1614026113
configPkg/: build-1614026113

build-1885967249: ../collector.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs1011/ccs/utils/sysconfig_1.6.0/sysconfig_cli.bat" -o "syscfg" --compiler ccs "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/error.h: build-1885967249 ../collector.syscfg
syscfg/: build-1885967249


