// FUN_00617960 @ 0x617960

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00617960(int param_1)

{
  int iVar1;
  
  FUN_00441b70(1);
  iVar1 = FUN_00402b30(*(undefined4 *)(param_1 + 0xda0));
  *(undefined4 *)(param_1 + 0xda4) = *(undefined4 *)(iVar1 + 0x1c);
  iVar1 = *(int *)(param_1 + 0xda4);
  if ((iVar1 == 0x41) || ((0x47 < iVar1 && (iVar1 < 0x4a)))) {
    *(undefined4 *)(param_1 + 0xde4) = _DAT_006983fc;
    FUN_004026b0("Confirm",0,0);
  }
  return;
}

