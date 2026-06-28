SECTIONS
{
  .rodata.svc_registry :
  {
    __start_svc_registry = .;
    KEEP(*(.rodata.svc_registry))
    __stop_svc_registry = .;
  } > DROM
}
