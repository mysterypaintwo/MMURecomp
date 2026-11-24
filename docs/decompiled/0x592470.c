// FUN_00592470 @ 0x592470

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00592470(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  float10 fVar3;
  float local_1c;
  undefined1 local_14 [16];
  
  FUN_00507e90(1);
  cVar1 = FUN_0041f990();
  if (cVar1 == '\x01') {
    local_1c = _DAT_00698468;
  }
  else {
    fVar3 = (float10)FUN_0043c800();
    local_1c = (float)fVar3;
  }
  *(float *)(param_1 + 0x280) = local_1c;
  FUN_004405c0("Stand",param_1 + 4);
  uVar2 = FUN_0043bc10(0,_DAT_00698d00);
  uVar2 = FUN_004014c0(local_14,uVar2);
  FUN_00403ef0(uVar2);
  FUN_00423920(param_1);
  return;
}

