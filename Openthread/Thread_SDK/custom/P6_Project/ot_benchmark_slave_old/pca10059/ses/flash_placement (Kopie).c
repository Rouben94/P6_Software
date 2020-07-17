<!DOCTYPE Linker_Placement_File>
<Root name="Flash Section Placement">
  <MemorySegment name="FLASH" start="$(FLASH_PH_START)" size="$(FLASH_PH_SIZE)">
    <ProgramSection alignment="0x100" load="Yes" name=".vectors" start="$(FLASH_START)" />
    <ProgramSection alignment="4" load="Yes" name=".init" />
    <ProgramSection alignment="4" load="Yes" name=".init_rodata" />
    <ProgramSection alignment="4" load="Yes" name=".text" />
    <ProgramSection alignment="4" keep="Yes" load="Yes" name=".pwr_mgmt_data" inputsections="*(SORT(.pwr_mgmt_data*))" address_symbol="__start_pwr_mgmt_data" end_symbol="__stop_pwr_mgmt_data" />
    <ProgramSection alignment="4" keep="Yes" load="Yes" name=".log_const_data" inputsections="*(SORT(.log_const_data*))" address_symbol="__start_log_const_data" end_symbol="__stop_log_const_data" />
    <ProgramSection alignment="4" keep="Yes" load="Yes" name=".log_backends" inputsections="*(SORT(.log_backends*))" address_symbol="__start_log_backends" end_symbol="__stop_log_backends" />
    <ProgramSection alignment="4" keep="Yes" load="Yes" name=".nrf_balloc" inputsections="*(.nrf_balloc*)" address_symbol="__start_nrf_balloc" end_symbol="__stop_nrf_balloc" />
    <ProgramSection alignment="4" keep="Yes" load="No" name=".nrf_sections" address_symbol="__start_nrf_sections" />
    <ProgramSection alignment="4" keep="Yes" load="Yes" name=".fs_data" inputsections="*(.fs_data*)" runin=".fs_data_run"/>
    <ProgramSection alignment="4" keep="Yes" load="Yes" name=".log_dynamic_data"  inputsections="*(SORT(.log_dynamic_data*))" runin=".log_dynamic_data_run"/>
    <ProgramSection alignment="4" keep="Yes" load="Yes" name=".log_filter_data"  inputsections="*(SORT(.log_filter_data*))" runin=".log_filter_data_run"/>
    <ProgramSection alignment="4" load="Yes" name=".dtors" />
    <ProgramSection alignment="4" load="Yes" name=".ctors" />
    <ProgramSection alignment="4" load="Yes" name=".rodata" />
    <ProgramSection alignment="4" load="Yes" name=".ARM.exidx" address_symbol="__exidx_start" end_symbol="__exidx_end" />
    <ProgramSection alignment="4" load="Yes" runin=".fast_run" name=".fast" />
    <ProgramSection alignment="4" load="Yes" runin=".data_run" name=".data" />
    <ProgramSection alignment="4" load="Yes" runin=".tdata_run" name=".tdata" />
  </MemorySegment>
  <MemorySegment name="RAM" start="$(RAM_PH_START)" size="$(RAM_PH_SIZE)">
    <ProgramSection alignment="0x100" load="No" name=".vectors_ram" start="$(RAM_START)" address_symbol="__app_ram_start__"/>
    <ProgramSection alignment="4" keep="Yes" load="No" name=".nrf_sections_run" address_symbol="__start_nrf_sections_run" />
    <ProgramSection alignment="4" keep="Yes" load="No" name=".fs_data_run" address_symbol="__start_fs_data" end_symbol="__stop_fs_data" />
    <ProgramSection alignment="4" keep="Yes" load="No" name=".log_dynamic_data_run" address_symbol="__start_log_dynamic_data" end_symbol="__stop_log_dynamic_data" />
    <ProgramSection alignment="4" keep="Yes" load="No" name=".log_filter_data_run" address_symbol="__start_log_filter_data" end_symbol="__stop_log_filter_data" />
    <ProgramSection alignment="4" keep="Yes" load="No" name=".nrf_sections_run_end" address_symbol="__end_nrf_sections_run" />
    <ProgramSection alignment="4" load="No" name=".fast_run" />
    <ProgramSection alignment="4" load="No" name=".data_run" />
    <ProgramSection alignment="4" load="No" name=".tdata_run" />
    <ProgramSection alignment="4" load="No" name=".bss" />
    <ProgramSection alignment="4" load="No" name=".tbss" />
    <ProgramSection alignment="4" load="No" name=".non_init" />
    <ProgramSection alignment="4" size="__HEAPSIZE__" load="No" name=".heap" />
    <ProgramSection alignment="8" size="__STACKSIZE__" load="No" place_from_segment_end="Yes" name=".stack"  address_symbol="__StackLimit" end_symbol="__StackTop"/>
    <ProgramSection alignment="8" size="__STACKSIZE_PROCESS__" load="No" name=".stack_process" />
  </MemorySegment>
  <MemorySegment name="ot_flash_data" start="0x27000" size="0x4000">
    <ProgramSection alignment="4" keep="Yes" load="Yes" name=".ot_flash_data" address_symbol="__start_ot_flash_data" end_symbol="__stop_ot_flash_data" start = "0x27000" size="0x4000" />
  </MemorySegment>
</Root>
