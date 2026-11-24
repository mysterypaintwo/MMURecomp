// FUN_00586310 @ 0x586310

void __fastcall FUN_00586310(int param_1)

{
  float fVar1;
  char cVar2;
  int iVar3;
  float *pfVar4;
  float10 fVar5;
  
  cVar2 = *(char *)(param_1 + 0x18c);
  fVar1 = *(float *)(param_1 + 400);
  fVar5 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0x58) =
       (float)(fVar5 * (float10)((float)(int)((-(uint)(cVar2 != '\0') & 2) - 1) * fVar1) +
              (float10)*(float *)(param_1 + 0x58));
  fVar5 = (float10)FUN_00585ff0(*(float *)(param_1 + 0x58) - *(float *)(param_1 + 0x198));
  if ((float10)*(float *)(param_1 + 0x194) < fVar5) {
    FUN_00462a90();
    iVar3 = FUN_00401930();
    fVar5 = (float10)FUN_00585ff0(*(float *)(param_1 + 0x5c) - *(float *)(iVar3 + 4));
    if (fVar5 <= (float10)*(float *)(param_1 + 0x194)) {
      FUN_00462a90();
      pfVar4 = (float *)FUN_00401930();
      fVar5 = (float10)FUN_00585ff0(*(float *)(param_1 + 0x58) - *pfVar4);
      if (fVar5 <= (float10)*(float *)(param_1 + 0x194)) {
        FUN_004405c0(&DAT_0067b1bc,param_1 + 4);
      }
      else {
        FUN_004405c0("MoveH",param_1 + 4);
      }
    }
    else {
      FUN_004405c0("MoveV",param_1 + 4);
    }
  }
  return;
}

