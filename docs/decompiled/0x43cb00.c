// FUN_0043cb00 @ 0x43cb00

float * FUN_0043cb00(void)

{
  int in_EAX;
  undefined4 uVar1;
  int iVar2;
  int local_c [3];
  
  uVar1 = FUN_00401930();
  uVar1 = FUN_00403d90(local_c,uVar1);
  FUN_00401510(uVar1);
  local_c[0] = FUN_006253a0();
  *(float *)(in_EAX + 0xac) = (float)local_c[0];
  iVar2 = FUN_006253a0();
  *(float *)(in_EAX + 0xb0) = (float)iVar2;
  return (float *)(in_EAX + 0xac);
}

