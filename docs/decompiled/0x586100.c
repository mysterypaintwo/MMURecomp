// FUN_00586100 @ 0x586100

void __fastcall FUN_00586100(int param_1)

{
  int iVar1;
  float *pfVar2;
  float10 fVar3;
  
  iVar1 = FUN_00462a90();
  if (iVar1 != 0) {
    FUN_00462a90();
    pfVar2 = (float *)FUN_00401930();
    fVar3 = (float10)FUN_00585ff0(*(float *)(param_1 + 0x58) - *pfVar2);
    if (fVar3 <= (float10)*(float *)(param_1 + 0x194)) {
      FUN_00462a90();
      iVar1 = FUN_00401930();
      fVar3 = (float10)FUN_00585ff0(*(float *)(param_1 + 0x5c) - *(float *)(iVar1 + 4));
      if ((float10)*(float *)(param_1 + 0x194) < fVar3) {
        FUN_004405c0("MoveV",param_1 + 4);
      }
    }
    else {
      FUN_004405c0("MoveH",param_1 + 4);
    }
  }
  return;
}

