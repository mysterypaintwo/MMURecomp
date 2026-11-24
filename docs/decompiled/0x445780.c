// FUN_00445770 @ 0x445780

void FUN_00445770(void)

{
  void *_Dst;
  void *_Dst_00;
  char cVar1;
  
  _Dst_00 = DAT_006c55c4;
  _Dst = (void *)((int)DAT_006c55c4 + 1200000);
  *(undefined4 *)((int)DAT_006c55c4 + 0x1e1c84) = 0;
  memset(_Dst,0,0xbcd00);
  memset(_Dst_00,0,1200000);
  cVar1 = FUN_00441c80(1,8);
  if (cVar1 != '\0') {
    *(bool *)((int)_Dst_00 + 0x1e1c80) = *(char *)((int)_Dst_00 + 0x1e1c80) == '\0';
  }
  return;
}

