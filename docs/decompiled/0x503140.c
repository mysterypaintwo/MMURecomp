// FUN_00503140 @ 0x503140

void __fastcall FUN_00503140(undefined4 param_1)

{
  undefined1 *puVar1;
  undefined4 uVar2;
  undefined1 local_14 [16];
  
  uVar2 = 1;
  FUN_00403240(1,param_1);
  FUN_00403220(uVar2);
  uVar2 = 0;
  FUN_00403240(0);
  FUN_00403220(uVar2);
  uVar2 = 1;
  FUN_00403240(1);
  FUN_00403220(uVar2);
  FUN_004026b0("Confirm",0,0);
  uVar2 = FUN_0043bc20(0x48,0x70);
  puVar1 = local_14;
  FUN_00401930(puVar1,uVar2);
  uVar2 = FUN_004014c0(puVar1,uVar2);
  FUN_004026e0(uVar2);
  return;
}

