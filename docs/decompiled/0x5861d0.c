// FUN_005861d0 @ 0x5861d0

void __fastcall FUN_005861d0(int param_1)

{
  float fVar1;
  char cVar2;
  float *pfVar3;
  int iVar4;
  float10 fVar5;
  
  cVar2 = *(char *)(param_1 + 0x18c);
  fVar1 = *(float *)(param_1 + 400);
  fVar5 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0x5c) =
       (float)(fVar5 * (float10)((float)(int)((-(uint)(cVar2 != '\0') & 0xfffffffe) + 1) * fVar1) +
              (float10)*(float *)(param_1 + 0x5c));
  fVar5 = (float10)FUN_00585ff0(*(float *)(param_1 + 0x5c) - *(float *)(param_1 + 0x19c));
  if ((float10)*(float *)(param_1 + 0x194) < fVar5) {
    FUN_00462a90();
    pfVar3 = (float *)FUN_00401930();
    fVar5 = (float10)FUN_00585ff0(*(float *)(param_1 + 0x58) - *pfVar3);
    if (fVar5 <= (float10)*(float *)(param_1 + 0x194)) {
      FUN_00462a90();
      iVar4 = FUN_00401930();
      fVar5 = (float10)FUN_00585ff0(*(float *)(param_1 + 0x5c) - *(float *)(iVar4 + 4));
      if (fVar5 <= (float10)*(float *)(param_1 + 0x194)) {
        FUN_004405c0(&DAT_0067b1d4,param_1 + 4);
      }
      else {
        FUN_004405c0("MoveV",param_1 + 4);
      }
    }
    else {
      FUN_004405c0("MoveH",param_1 + 4);
    }
  }
  return;
}

