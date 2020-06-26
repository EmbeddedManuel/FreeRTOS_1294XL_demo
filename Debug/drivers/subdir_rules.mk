################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
drivers/%.obj: ../drivers/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"W:/Development/TI/CCS/ccs/tools/compiler/ti-cgt-arm_20.2.0.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/m009b/Documents/TI/default_workspace/FreeRTOS_1294XL_demo" --include_path="C:/Users/m009b/Documents/TI/default_workspace/FreeRTOS_1294XL_demo/FreeRTOS-Utils" --include_path="C:/Users/m009b/Documents/TI/default_workspace/FreeRTOS_1294XL_demo/drivers" --include_path="C:/Users/m009b/Documents/TI/default_workspace/FreeRTOS_1294XL_demo/TivaWare" --include_path="C:/Users/m009b/Documents/TI/default_workspace/FreeRTOS_1294XL_demo/FreeRTOS/portable/CCS/ARM_CM4F" --include_path="W:/Development/TI/CCS/ccs/tools/compiler/ti-cgt-arm_20.2.0.LTS/include" --include_path="C:/Users/m009b/Documents/TI/default_workspace/FreeRTOS_1294XL_demo/FreeRTOS" --include_path="C:/Users/m009b/Documents/TI/default_workspace/FreeRTOS_1294XL_demo/FreeRTOS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="drivers/$(basename $(<F)).d_raw" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


