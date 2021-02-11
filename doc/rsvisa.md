# Current limitations

* `viLock()`, `viUnlock()` are implemented for the VXI-11 and HiSLIP
  protocols. For other resources, e.g. raw socket and USB, only
  reasonable results are returned without locking.
* Asynchronous IO, e.g. `viReadAsync()` and `viWriteAsync()`, is not
  available. These functions return the error code `VI_ERROR_NSUP_OPER`.
* The Memory IO functions, e.g. `viIn8/16/32()`, `viOut8/16/32()`,
  `viMove`, ..., `viPeek8/16/32()`, are VXI specific. For all other
  instruments these functions return the error code `VI_ERROR_NSUP_OPER`.
* Direct communication with the USB device via `viUsbControlIn()` and
  `viUsbControlOut()` is not supported.
