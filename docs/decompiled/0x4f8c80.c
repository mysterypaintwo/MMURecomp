// FUN_004f8c80 @ 0x4f8c80

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f8c80(int param_1)

{
  float fVar1;
  float fVar2;
  int iVar3;
  uint *puVar4;
  undefined4 uVar5;
  float10 fVar6;
  uint uVar7;
  
  fVar6 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0x3478) = (float)((float10)*(float *)(param_1 + 0x3478) - fVar6);
  fVar1 = *(float *)(param_1 + 0x3478);
  fVar2 = (float)_DAT_00697ce0;
  if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
    iVar3 = FUN_00462a90();
    puVar4 = (uint *)(iVar3 + 0x150);
    *(undefined4 *)(param_1 + 0x3478) = _DAT_006989cc;
    *puVar4 = *puVar4 + 1;
    uVar7 = *puVar4;
    FUN_004ac790(uVar7);
    FUN_00420200(uVar7);
    if (0x1b < *puVar4) {
      FUN_00462a90();
      iVar3 = FUN_00404750();
      *(int *)(param_1 + 0x3470) = iVar3 / 7;
      *(int *)(param_1 + 0x3474) = iVar3 % 7;
      FUN_00433140();
      CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
      uVar5 = FUN_00430cd0();
      FUN_00623ff0(uVar5);
      FUN_00420a90("EnergyMeter3s");
      FUN_004405c0("SelectItem",param_1 + 4);
      FUN_00441b70(0);
    }
  }
  return;
}

