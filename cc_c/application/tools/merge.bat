del release.bin
create release.bin 20480 
cat .\..\Application\Obj\project.bin>>release.bin
hbin .\..\IAP_BootFlash\Obj\BootFlash.bin release.bin